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

#include "MpCCIDataHandler.h"
#include "MpCCIMeshData.h"

#include <iosfwd>
#include <string_view>
#include <vector>

class SIMinput;
struct TimeDomain;

namespace MpCCI {


//! \brief Class handling a MpCCI job.
//! \details Only a single instance is allowed.
class Job
{
public:
  static Job* globalInstance; //!< Singleton static pointer
  static bool dryRun; //!< To perform a dry run - used in tests

  //! \brief The constructor initializes the MpCCI job.
  Job(SIMinput& simulator, const double dt,
      DataHandler* hndler = nullptr,
      GlobalHandler* ghndler = nullptr);

  //! \brief The destructor deinitializes the MpCCI job.
  ~Job();

  //! \brief Called during MpCCI_Init.
  //! \details Used to defines nodes and elements
  static int definePart(MPCCI_SERVER* server, MPCCI_PART* part);

  //! \brief Called to update part info / quant info.
  static int partUpdate(MPCCI_PART* part, MPCCI_QUANT* quant);

  //! \brief Called to transfer data to MpCCI server.
  static int getFaceNodeValues(const MPCCI_PART* part,
                               const MPCCI_QUANT* quant,
                               void* values);

  //! \brief Called to transfer from MpCCI server.
  static void putFaceNodeValues(const MPCCI_PART* part,
                                const MPCCI_QUANT* quant,
                                void* values);

  static int getGlobalValues (const MPCCI_GLOB* glob, void* values);
  static void putGlobalValues (const MPCCI_GLOB* glob, void* values);

  //! \brief Execute data transfer.
  int transfer(int status, TimeDomain& time);

  void done();

private:
  MPCCI_JOB* mpcciJob{nullptr}; //!< MpCCI job info structure
  MPCCI_TINFO mpcciTinfo{0}; //!< MpCCI time step information
  MeshInfo meshInfo; //!< Coupling mesh info
  SIMinput& sim; //!< Reference to IFEM simulator
  DataHandler* handler; //!< Data handler
  GlobalHandler* ghandler; //!< Global data handler
};

}

#endif
