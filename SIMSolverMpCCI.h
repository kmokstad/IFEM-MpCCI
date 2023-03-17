// $Id$
//==============================================================================
//!
//! \file SIMSolver.h
//!
//! \date Oct 12 2012
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief SIM solver class template.
//!
//==============================================================================

#ifndef _SIM_SOLVER_MPCCI_H_
#define _SIM_SOLVER_MPCCI_H_

#include "NewmarkSIM.h"
#include "SIMSolver.h"
#include "MpCCIJob.h"


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
    MpCCI::Job job(this->S1, &this->S1, this);

    int status = MPCCI_CONV_STATE_CONTINUE;
    tp.step = 0;
    tp.time.t = 0.0;
    tp.time.dt = 2.5e-4;
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

template<class T1> class SIMSolver : public ::SIMSolver<T1>
{
public:
  //! \brief The constructor initializes the reference to the actual solver.
  explicit SIMSolver(T1& s1) : ::SIMSolver<T1>(s1)
  {
  }

  //! \brief Parses a data section from an XML element.
  bool parse(const TiXmlElement* elem) override
  {
    if (!strcasecmp(elem->Value(),"newmarksolver"))  {
      const TiXmlElement* child = elem->FirstChildElement();
      for (; child; child = child->NextSiblingElement())
        this->tp.parse(child);
    }

    return this->::SIMSolver<T1>::parse(elem);
  }

  //! \brief Solves the problem up to the final time.
  int solveProblem(char* infile, const char* heading = nullptr) override
  {
    if (!this->read(infile))
      return 5;

    if (this->S1.opt.dumpHDF5(infile))
      this->handleDataOutput(this->S1.opt.hdf5,this->S1.getProcessAdm());

    // Save FE model to VTF and HDF5 for visualization.
    // Save the initial configuration also if multi-step simulation.
    int geoBlk = 0, nBlock = 0;
    if (!this->saveState(geoBlk,nBlock,true,infile,this->tp.multiSteps()))
      return 2;

    this->printHeading(heading);

    MpCCI::Job job(this->S1, &this->S1, nullptr);
    NewmarkSIM nSim(this->S1);
    nSim.initPrm();
    nSim.initSolution(this->S1.getNoDOFs(), 3);

    // Solve for each time step up to final time
    int status = MPCCI_CONV_STATE_CONVERGED;
    this->tp.step = 0;
    this->tp.time.t = 0.0;
    this->tp.time.dt = 2.5e-4;
    this->S1.initLHSbuffers();
    while (((status = job.transfer(status, this->tp.time)) == MPCCI_CONV_STATE_CONTINUE ||
            status == MPCCI_CONV_STATE_CONVERGED) && this->advanceStep()) {
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
};

}

#endif
