// $Id$
//==============================================================================
//!
//! \file MpCCIJob.h
//!
//! \date Feb 23 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief The IFEM MpCCI adapter job class.
//!
//==============================================================================

#ifndef MPCCIJOB_H_
#define MPCCIJOB_H_

#include <mpcci.h>

class SIMbase;

//! \brief Class handling a MpCCI job
//! \details Only a single instance is allowed.
class MpCCIJob
{
public:
  //! \brief The constructor initializes the MpCCI job.
  MpCCIJob(SIMbase& simulator);

  //! \brief The destructor deinitializes the MpCCI job.
  ~MpCCIJob();

private:
  MPCCI_JOB* mpcciJob{nullptr};
  MPCCI_TINFO mpcciTinfo{0};
  SIMbase& sim;
};

#endif
