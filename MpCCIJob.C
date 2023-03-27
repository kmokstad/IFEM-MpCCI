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
#include "SIMinput.h"

#include <mpcci.h>

#include <cstdlib>
#include <stdexcept>

namespace {

void exit_mpcci_job ()
{
    //std::terminate();
}


void mpcci_printer (const char* s, int len)
{
  if (len && s)
    IFEM::cout << s;
}

}


namespace MpCCI {

Job* Job::globalInstance = nullptr;
bool Job::dryRun = false;


Job::Job (SIMinput& simulator, const double dt,
          DataHandler* hndler, GlobalHandler* ghndler) :
  sim(simulator), handler(hndler), ghandler(ghndler)
{
  globalInstance = this;
  if (dryRun)
    return;

  static MPCCI_DRIVER driver =
  {
    (unsigned)sizeof(MPCCI_DRIVER),     /* this_size: sizeof(this) */
    MPCCI_CCM_VERSION,                  /* this_version */
    0,                                  /* tact_required bitmask */
    {                                   /* part_description */
       "node",
       "edge",
       "face",
       "patch",
       "*BAD*",
       "global value",
       nullptr
    },
    static_cast<unsigned>(sizeof(double)),
    nullptr,                            // afterCloseSetup()
    nullptr,                            // beforeGetAndSend()
    nullptr,                            // afterGetAndSend()
    nullptr,                            // beforeRecvAndPut()
    nullptr,                            // afterRecvAndPut()
    nullptr,                            // partSelect()
    partUpdate,                         // partUpdate()
    nullptr,                            // appendParts()
    nullptr,                            // getPartRemeshState()
    definePart,                         // definePart(
    nullptr,                            // partInfo()
    nullptr,                            // getNodes()
    nullptr,                            // getElems()
    nullptr,                            // moveNodes()
    nullptr,                            // getPointValues()
    nullptr,                            // getLineNodeValues()
    nullptr,                            // getLineElemValues()
    getFaceNodeValues,                  // getFaceNodeValues()
    getFaceNodeValues,                  // getFaceElemValues()
    nullptr,                            // getVoluNodeValues()
    nullptr,                            // getVoluElemValues()
    getGlobalValues,                    // getGlobalValues()
    nullptr,                            // putPointValues()
    nullptr,                            // putLineNodeValues()
    nullptr,                            // putLineElemValues()
    putFaceNodeValues,                  // putFaceNodeValues()
    putFaceNodeValues,                  // putFaceElemValues()
    nullptr,                            // putVolumNodeValues()
    nullptr,                            // putVolumElemValues()
    putGlobalValues                     // putGlobalValues()
  };

  MPCCI_CINFO cinfo;

  umpcci_msg_functs(mpcci_printer, mpcci_printer, mpcci_printer,
                    nullptr, nullptr, nullptr, exit_mpcci_job);

  umpcci_msg_prefix("MpCCI: ");
  umpcci_msg_level(MPCCI_MSG_LEVEL_DEBUG);
  atexit(exit_mpcci_job);

  ampcci_tinfo_init(&mpcciTinfo, nullptr);
  mpcciTinfo.mpcci_state = mpcciTinfo.mpcci_used = 0;
  mpcciTinfo.iter = -1;
  mpcciTinfo.time = 0;
  mpcciTinfo.dt = dt;

  mpcci_cinfo_init(&cinfo, &mpcciTinfo);
  cinfo.codename = "ifem";
  cinfo.flags    = MPCCI_CFLAG_TYPE_FEA|MPCCI_CFLAG_GRID_CURR;
  cinfo.nclients = 1;
  cinfo.nprocs   = 1;
  cinfo.time = 0;

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


Job::~Job ()
{
  if (mpcciJob)
    mpcci_quit(&mpcciJob);
}


int Job::definePart (MPCCI_SERVER* server, MPCCI_PART* part)
{
  MeshInfo& info = globalInstance->meshInfo;
  info = meshData(part->name, globalInstance->sim);

  if (!globalInstance->handler->addCoupling(part->name, info)) {
    MPCCI_MSG_INFO0("Failed to establish coupling property.\n");
    return 1;
  }

   MPCCI_MSG_INFO1("Coupling grid definition for component \"%s\" ...\n",
                   MPCCI_PART_NAME(part));
   MPCCI_MSG_INFO1("We have %i nodes\n", int(info.nodes.size()));
   MPCCI_MSG_INFO1("We have %i elms\n", int(info.elms.size() / info.node_per_elm));

   MPCCI_PART_NNODES(part) = info.nodes.size();
   MPCCI_PART_NELEMS(part) = info.elms.size() / info.node_per_elm;

   int ret = smpcci_defp(server,
                         MPCCI_PART_MESHID(part),
                         MPCCI_PART_PARTID(part),
                         MPCCI_CSYS_C3D,
                         info.nodes.size(),
                         info.elms.size() / info.node_per_elm,
                         MPCCI_PART_NAME  (part));

   // Send the nodes definition for this coupled component
   ret = smpcci_pnod(server,
                     MPCCI_PART_MESHID(part),  /* mesh id */
                     MPCCI_PART_PARTID(part),  /* part id */
                     MPCCI_CSYS_C3D,
                     info.nodes.size(),  /* no. of nodes */
                     info.coords.data(),               /* node coordinates */
                     static_cast<unsigned>(sizeof(double)),  /* data type of coordinates */
                     info.nodes.data(),              /* node ids */
                     nullptr);

   // Send the element definiton for this coupled component
   ret = smpcci_pels(server,
                     MPCCI_PART_MESHID(part),    /* mesh id */
                     MPCCI_PART_PARTID(part),    /* part id */
                     info.elms.size() / info.node_per_elm,  /* number of elements */
                     info.type,               /* first element type */
                     nullptr,                  /* element types */
                     info.elms.data(),                  /* nodes of the element */
                     nullptr);                      /* element IDs */

  return ret;
}


int Job::getFaceNodeValues (const MPCCI_PART* part,
                            const MPCCI_QUANT* quant,
                            void* values)
{
  if (MPCCI_QUANT_SMETHOD(quant) != MPCCI_QSM_DIRECT)
    throw std::runtime_error("Invalid quantity method requested in getFaceNodeValues " +
                             std::to_string(MPCCI_QUANT_SMETHOD(quant)));

  if (globalInstance->handler)
    globalInstance->handler->writeData(MPCCI_QUANT_QID(quant),
                                       globalInstance->meshInfo,
                                       static_cast<double*>(values));

   MPCCI_MSG_INFO0("finished send values...\n");

   return sizeof(double); /* return the size of the value data type */
}


void Job::putFaceNodeValues (const MPCCI_PART* part,
                             const MPCCI_QUANT* quant,
                             void* values)
/*****************************************************************************/
{
   MPCCI_MSG_INFO0("entered receive values...\n");

   /* check whether this is the appropriate method */

   MPCCI_MSG_ASSERT(MPCCI_PART_IS_FACE(part));

  if (MPCCI_QUANT_SMETHOD(quant) != MPCCI_QSM_DIRECT)
    throw std::runtime_error("Invalid quantity method requested in putFaceNodeValues " +
                             std::to_string(MPCCI_QUANT_SMETHOD(quant)));

  if (globalInstance->handler)
    globalInstance->handler->readData(MPCCI_QUANT_QID(quant),
                                     globalInstance->meshInfo,
                                     static_cast<double*>(values));

  MPCCI_MSG_INFO0("finished receive values...\n");
}


int Job::partUpdate(MPCCI_PART* part,
                    MPCCI_QUANT* quant)
{
  MPCCI_PART_NNODES(part) = globalInstance->meshInfo.nodes.size();
  MPCCI_PART_NELEMS(part) = globalInstance->meshInfo.elms.size() / 4;
  quant->flags &= ~MPCCI_QFLAG_LOC_MASK;
  quant->flags |= (MPCCI_QUANT_IS_COORD(quant) ? MPCCI_QFLAG_LOC_VERT : MPCCI_QFLAG_LOC_CELL);
  return 0;
}


int Job::transfer(int status, TimeDomain& time)
{
  mpcciTinfo.time = time.t;
  mpcciTinfo.dt = time.dt;
  mpcciTinfo.iter = -1;
  mpcciTinfo.conv_code = status;

  umpcci_conv_cstate(mpcciTinfo.conv_code);
  int ret = ampcci_transfer(mpcciJob, &mpcciTinfo);
  if (ret == -1) {
    MPCCI_MSG_WARNING0("Error during data transfer: Check log file\n");
    throw std::runtime_error("Error during data transfer. Check log file");
  } else if (ret == 0) {
    MPCCI_MSG_INFO0("No data transfer ocurred.\n");
    return MPCCI_CONV_STATE_INVALID;
  }

  return mpcciTinfo.conv_job;
}


void Job::done()
{
  umpcci_conv_cstate(MPCCI_CONV_STATE_STOP);
}


int Job::getGlobalValues (const MPCCI_GLOB* glob, void* values)
{
  double* valptr = static_cast<double*>(values);
  if (globalInstance->ghandler)
    globalInstance->ghandler->writeGlobal(MPCCI_QUANT_QID(glob), valptr);
  return sizeof(double);
}


void Job::putGlobalValues (const MPCCI_GLOB* glob, void* values)
{
  double* valptr = static_cast<double*>(values);
  if (globalInstance->ghandler)
    globalInstance->ghandler->readGlobal(MPCCI_QUANT_QID(glob), valptr);
}

}
