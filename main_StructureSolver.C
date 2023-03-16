// $Id$
//==============================================================================
//!
//! \file main_MpCCI.C
//!
//! \date Feb 23 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Main program for the IFEM MpCCI adapter.
//!
//==============================================================================

#include "MpCCIArgs.h"
#include "MpCCIJob.h"
#include "SIMMpCCIStructure.h"

#include "IFEM.h"
#include "NewmarkDriver.h"
#include "NewmarkSIM.h"
#include "Profiler.h"
#include "SIM3D.h"
#include "SIMSolver.h"

#include <iostream>
#include <numeric>
#include <stdexcept>


/*!
  \brief Main program for the structure solver in the MpCCI adapter.

  The input to the program is specified through the following
  command-line arguments. The arguments may be given in arbitrary order.
*/

int main (int argc, char** argv)
{
  Profiler prof(argv[0]);
  utl::profiler->start("Initialization");
  char* infile = nullptr;
  IFEM::Init(argc,argv,"Structure solver");
  MpCCIArgs args;
  for (int i = 1; i < argc; ++i) {
    if (argv[i] == infile || args.parseArg(argv[i]))
      ;
    else if (SIMoptions::ignoreOldOptions(argc,argv,i))
      ;
    else if (!infile) {
      infile = argv[i];
      if (!args.readXML(infile,false))
        return 1;
      i = 0;
    }
  }

  if (!infile) {
    std::cout << "usage: " << argv[0] << " <inputfile>\n";
    return 1;
  }

  utl::profiler->stop("Initialization");

  MpCCI::SIMStructure<SIM3D> sim;

  if (args.newmark) {
    NewmarkDriver<NewmarkSIM> solver(sim);
    solver.initPrm();
    if (!solver.read(infile))
      return 5;

    if (!sim.preprocess())
      return 3;

    if (!sim.initSystem(sim.opt.solver))
      return 4;

    sim.opt.print(IFEM::cout,true) << std::endl;

    solver.initSolution(sim.getNoDOFs(), 3);
    solver.initPrm();
    std::unique_ptr<DataExporter> exporter;
    if (sim.opt.dumpHDF5(infile)) {
      exporter = std::make_unique<DataExporter>(true);
      exporter->registerWriter(new HDF5Writer(sim.opt.hdf5,sim.getProcessAdm()));
      int flag = DataExporter::PRIMARY;
      exporter->registerField("u","solution",DataExporter::SIM,flag);
      exporter->setFieldValue("u",&sim,&solver.getSolution());
    }

    return solver.solveProblem(exporter.get(), nullptr);
  } else {
    if (!sim.read(infile))
      return 5;

    if (!sim.preprocess())
      return 3;

    if (!sim.initSystem(sim.opt.solver))
      return 4;
    sim.initSolution(sim.getNoDOFs());

    MpCCI::Job::dryRun = true;
    MpCCI::Job job(sim, sim);
    auto info = job.meshData("couple-flap");
    sim.addCoupling("couple-flap", info);
    std::vector<double> values(info.gelms.size());
    double val = 1e6;
    for (double& d : values) {
      d = val; val += 1e6;
    }
    sim.readData(MPCCI_QID_ABSPRESSURE, info, values.data());

    SIMSolverStat solver(sim);
    if (!solver.read(infile))
      return 5;

    if (sim.opt.dumpHDF5(infile))
      solver.handleDataOutput(sim.opt.hdf5,sim.getProcessAdm());

    return solver.solveProblem(infile, "Solving static structure problem");
  }

  return 0;
}

