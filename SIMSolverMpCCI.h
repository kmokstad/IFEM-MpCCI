// $Id$
//==============================================================================
//!
//! \file SIMSolverMpCCI.h
//!
//! \date Mar 28 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief MpCCI solver class template.
//!
//==============================================================================

#ifndef _SIM_SOLVER_MPCCI_H_
#define _SIM_SOLVER_MPCCI_H_

#include "NewmarkSIM.h"
#include "SIMSolver.h"
#include "MpCCIMockJob.h"
#include "MpCCIJob.h"
#include "Utilities.h"


namespace MpCCI {

/*!
  \brief Template class for stationary simulator drivers.
  \details This template can be instantiated over any type implementing the
  ISolver interface. It provides data output to HDF5 and VTF.
*/

template<class T1> class SIMSolverStat : public ::SIMSolverStat<T1>,
                                         public MpCCI::GlobalHandler
{
public:
  //! \brief The constructor initializes the reference to the actual solver.
  explicit SIMSolverStat(T1& s1, const char* head = nullptr) :
    ::SIMSolverStat<T1>(s1, head)
  {
  }

  //! \brief Solves the stationary problem.
  int solveProblem(char* infile, const char* heading = nullptr) override
  {
    // Save FE model to VTF for visualization
    int geoBlk = 0, nBlock = 0;
    if (!this->S1.saveModel(infile,geoBlk,nBlock))
      return 1;

    this->printHeading(heading);

    // Start the simulation
    MpCCI::Job job(this->S1, 1.0e-4, &this->S1, this);

    int status = MPCCI_CONV_STATE_CONTINUE;
    tp.step = 0;
    tp.time.t = 0.0;
    tp.time.dt = 1.0e-4;
    while ((status = job.transfer(status, tp.time)) == MPCCI_CONV_STATE_CONTINUE ||
           status == MPCCI_CONV_STATE_CONVERGED) {
      this->S1.printStep(tp.step, tp.time);
      if (!this->S1.solveStep(tp))
        return 2;

      // Save the results
      if (!this->S1.saveStep(tp,nBlock))
        return 4;

      if (this->exporter && !this->exporter->dumpTimeLevel())
        return 5;

      ++tp.step;
      tp.time.t += tp.time.dt;
    }

    return 0;
  }

  void readGlobal(int quant_id, const double* data) override
  {
    if (quant_id == MPCCI_QID_PHYSICAL_TIME)
      tp.time.t = *data;
    else if (quant_id == MPCCI_QID_TIMESTEP_SIZE)
      tp.time.dt = *data;
  }

  void writeGlobal(int quant_id, double* data) const override
  {
    if (quant_id == MPCCI_QID_PHYSICAL_TIME)
      *data = tp.time.t;
    else if (quant_id == MPCCI_QID_TIMESTEP_SIZE)
      *data = tp.time.dt;
  }

private:
  TimeStep tp;
};


/*!
  \brief Template class for transient simulator drivers.
  \details This template can be instantiated over any type implementing the
  ISolver interface. It provides a time stepping loop and restart in addition.
*/

template<class T1, class Newmark = NewmarkSIM, class Job = MpCCI::Job>
class SIMSolver : public ::SIMSolver<T1>
{
public:
  //! \brief The constructor initializes the reference to the actual solver.
  explicit SIMSolver(T1& s1) : ::SIMSolver<T1>(s1)
  {
  }

  //! \brief Parses a data section from an XML element.
  bool parse(const tinyxml2::XMLElement* elem) override
  {
    if (!strcasecmp(elem->Value(), "mpcci"))  {
      const tinyxml2::XMLElement* child = elem->FirstChildElement();
      for (; child; child = child->NextSiblingElement())
        if (!strcasecmp(child->Value(),"saveData"))
          mpcciSerializer = std::make_unique<HDF5Restart>(couplingFile,
                                                          this->S1.getProcessAdm());
        else if (!strcasecmp(child->Value(),"couplingSet"))
          couplingSet = utl::getValue(child, "couplingSet");

      return true;
    }
    if (!strcasecmp(elem->Value(),"newmarksolver"))  {
      const tinyxml2::XMLElement* child = elem->FirstChildElement();
      for (; child; child = child->NextSiblingElement())
        this->tp.parse(child);
    }

    return this->::SIMSolver<T1>::parse(elem);
  }

  //! \brief Solves the problem up to the final time.
  int solveProblem(char* infile, const char* heading = nullptr) override
  {
    couplingFile = infile;
    couplingFile.erase(couplingFile.find_last_of("."));
    couplingFile += "_mpcci_data";

    Newmark nSim(this->S1);
    if (!nSim.read(infile) || !this->read(infile))
      return 2;

    if (!this->S1.preprocess())
      return 3;

    if (!nSim.initEqSystem(false))
      return 4;

    nSim.initSol(3);
    nSim.initPrm();
    this->S1.initSolution(this->S1.getNoDOFs(), 1);
    nSim.printProblem();

    if (this->S1.opt.dumpHDF5(infile))
      this->handleDataOutput(this->S1.opt.hdf5,this->S1.getProcessAdm());

    // Save FE model to VTF and HDF5 for visualization.
    // Save the initial configuration also if multi-step simulation.
    int geoBlk = 0, nBlock = 0;
    if (!this->saveState(geoBlk,nBlock,true,infile,this->tp.multiSteps()))
      return 2;

    Job job(this->S1, this->tp.time.dt, &this->S1, nullptr);

    if constexpr (std::is_same_v<Job, MpCCI::MockJob>) {
      job.setInputFile(couplingFile, couplingSet, this->S1);
      mpcciSerializer.reset();
    }

    this->printHeading(heading);

    // Solve for each time step up to final time
    int status = MPCCI_CONV_STATE_CONVERGED;
    this->S1.initLHSbuffers();

    while (((status = job.transfer(status, this->tp.time)) == MPCCI_CONV_STATE_CONTINUE ||
            status == MPCCI_CONV_STATE_CONVERGED) && this->advanceStep()) {
      nSim.advanceStep(this->tp, false);
      if (nSim.solveStep(this->tp, SIM::DYNAMIC) != SIM::CONVERGED) {
        job.transfer(MPCCI_CONV_STATE_DIVERGED, this->tp.time);
        return 3;
      }
      this->S1.setSolution(nSim.getSolution(), 0);

      if (!this->saveState(geoBlk,nBlock)) {
        job.transfer(MPCCI_CONV_STATE_DIVERGED, this->tp.time);
        return 4;
      } else
        IFEM::pollControllerFifo();

      if (this->tp.time.t == this->tp.stopTime)
        break;
    }

    job.done();
    return 0;
  }

  //! \brief Extends data output with serialization of MpCCI pressure loads.
  bool saveState(int& geoBlk, int& nBlock, bool newMesh = false,
                 char* infile = nullptr, bool saveRes = true)
  {
    if (mpcciSerializer) {
      HDF5Restart::SerializeData data;
      this->S1.serializeMpCCIData(data);
      this->mpcciSerializer->writeData(data);
    }

    return this->::SIMSolver<T1>::saveState(geoBlk, nBlock,
                                            newMesh, infile, saveRes);
  }

protected:
  std::unique_ptr<HDF5Restart> mpcciSerializer; //!< Serializer for MpCCI coupling data
  std::string couplingSet; //!< Name of set used for coupling, used when running with mocked MpCCI
  std::string couplingFile; //!< Name of file used for coupling data
};


}

#endif
