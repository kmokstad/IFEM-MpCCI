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


/*!
  \brief Template class for stationary simulator drivers.
  \details This template can be instantiated over any type implementing the
  ISolver interface. It provides data output to HDF5 and VTF.
*/

template<class T1> class SIMSolverMpCCIStat : public SIMSolverStat<T1>
{
public:
  //! \brief The constructor initializes the reference to the actual solver.
  explicit SIMSolverMpCCIStat(T1& s1, const char* head = nullptr) :
    SIMSolverStat<T1>(s1, head)
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
    MpCCI::Job job(this->S1, this->S1);

    int status = MPCCI_CONV_STATE_CONTINUE;
    while ((status = job.transfer(status)) == MPCCI_CONV_STATE_CONTINUE || status == MPCCI_CONV_STATE_CONVERGED) {
/*      TimeStep dummy;
      if (!this->S1.solveStep(dummy))
        return 2;

      // Save the results
      if (!this->S1.saveStep(dummy,nBlock))
        return 4;

      if (this->exporter && !this->exporter->dumpTimeLevel())
        return 5;*/
    }

    return 0;
  }
};


/*!
  \brief Template class for transient simulator drivers.
  \details This template can be instantiated over any type implementing the
  ISolver interface. It provides a time stepping loop and restart in addition.
*/

template<class T1> class SIMSolverMpCCI : public SIMSolver<T1>
{
public:
  //! \brief The constructor initializes the reference to the actual solver.
  explicit SIMSolverMpCCI(T1& s1) : SIMSolver<T1>(s1)
  {
  }

  //! \brief Solves the problem up to the final time.
  int solveProblem(char* infile, const char* heading = nullptr) override
  {
    // Save FE model to VTF and HDF5 for visualization.
    // Save the initial configuration also if multi-step simulation.
    int geoBlk = 0, nBlock = 0;
    if (!this->saveState(geoBlk,nBlock,true,infile,this->tp.multiSteps()))
      return 2;

    this->printHeading(heading);

    MpCCI::Job job(this->S1, this->S1);
    NewmarkSIM nSim(this->S1);
    nSim.initPrm();
    nSim.initSolution(this->S1.getNoDOFs(), 3);

    // Solve for each time step up to final time
    int status = MPCCI_CONV_STATE_CONVERGED;
    for (int iStep = 1; (status = job.transfer(status)) != MPCCI_CONV_STATE_CONTINUE && this->advanceStep(); iStep++)
      if (nSim.solveStep(this->tp, SIM::DYNAMIC) != SIM::CONVERGED) {
        job.transfer(MPCCI_CONV_STATE_DIVERGED);
        return 3;
      } else if (!this->saveState(geoBlk,nBlock)) {
        job.transfer(MPCCI_CONV_STATE_DIVERGED);
        return 4;
      } else
        IFEM::pollControllerFifo();

    return 0;
  }
};

#endif
