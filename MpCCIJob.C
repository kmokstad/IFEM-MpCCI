// $Id$
//==============================================================================
//!
//! \file MpCCIJob.C
//!
//! \date Feb 23 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief The IFEM MpCCI adapter job class.
//!
//==============================================================================
#include "MpCCIJob.h"

#include "IFEM.h"

#include <mpcci.h>

#include <cstdlib>
#include <stdexcept>

namespace {

void exit_mpcci_job ()
{
    std::terminate();
}

void mpcci_printer (const char* s, int len)
{
  if (len && s)
    IFEM::cout << s;
}

}


MpCCIJob::MpCCIJob (SIMbase& simulator) :
  sim(simulator)
{
  static MPCCI_DRIVER driver =
  {
    (unsigned)sizeof(MPCCI_DRIVER),     /* this_size: sizeof(this) */
    MPCCI_CCM_VERSION,                  /* this_version */
    0,                                  /* tact_required bitmask */
    {                                   /* part_description */
       "POINT",
       "*BAD*",
       "*BAD*",
       "*BAD*",
       "*BAD*",
       "GLOBAL",
       nullptr
    },
    static_cast<unsigned>(sizeof(double)),
    nullptr,                            // afterCloseSetup()
    nullptr,                            // beforeGetAndSend()
    nullptr,                            // afterGetAndSend()
    nullptr,                            // beforeRecvAndPut()
    nullptr,                            // afterRecvAndPut()
    nullptr,                            // partSelect()
    nullptr,                            // partUpdate()
    nullptr,                            // appendParts()
    nullptr,                            // getPartRemeshState()
    nullptr,                            // definePart(
    nullptr,                            // partInfo()
    nullptr,                            // getNodes()
    nullptr,                            // getElems()
    nullptr,                            // moveNodes()
    nullptr,                            // getPointValues()
    nullptr,                            // getLineNodeValues()
    nullptr,                            // getLineElemValues()
    nullptr,                            // getFaceNodeValues()
    nullptr,                            // getFaceElemValues()
    nullptr,                            // getVoluNodeValues()
    nullptr,                            // getVoluElemValues()
    nullptr,                            // getGlobalValues()
    nullptr,                            // putPointValues()
    nullptr,                            // putLineNodeValues()
    nullptr,                            // putLineElemValues()
    nullptr,                            // putFaceNodeValues()
    nullptr,                            // putFaceElemValues()
    nullptr,                            // putVolumNodeValues()
    nullptr,                            // putVolumElemValues()
    nullptr                             // putGlobalValues()
  };

  MPCCI_CINFO cinfo;

  umpcci_msg_functs(mpcci_printer, mpcci_printer, mpcci_printer,
                    nullptr, nullptr, nullptr, exit_mpcci_job);

  umpcci_msg_prefix("MpCCI: ");
  umpcci_msg_level(MPCCI_MSG_LEVEL_DEBUG);
  atexit(exit_mpcci_job);

  ampcci_tinfo_init(&mpcciTinfo, nullptr);
  mpcciTinfo.mpcci_state = mpcciTinfo.mpcci_used = 0;
  mpcciTinfo.iter = 0;
  mpcciTinfo.dt   = 0.0;
  mpcciTinfo.time = 0.0;

  mpcci_cinfo_init(&cinfo, &mpcciTinfo);
  cinfo.codename = "IFEM-MpCCI";
  cinfo.flags    = MPCCI_CFLAG_TYPE_SBM|MPCCI_CFLAG_GRID_CURR;
  cinfo.nclients = 1;
  cinfo.nprocs   = 1;

  mpcciJob       = mpcci_init(nullptr, &cinfo);
  if (mpcciJob) {
    int ret = ampcci_config(&mpcciJob, &driver);
    if (ret < 0) {
      mpcci_quit(&mpcciJob);
      throw std::runtime_error("Error initializing MpCCI");
    }
  }

  mpcciTinfo.mpcci_used = (mpcciJob != nullptr);
  if (!mpcciTinfo.mpcci_used) {
    throw std::runtime_error("This process is not an MpCCI process.");
  }

  mpcciTinfo.mpcci_state = 1;
}


MpCCIJob::~MpCCIJob ()
{
  if (mpcciJob)
    mpcci_quit(&mpcciJob);
}
