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

#include "ASMbase.h"
#include "IFEM.h"
#include "SIMinput.h"

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

namespace MpCCI {

Job* Job::globalInstance = nullptr;
bool Job::dryRun = false;


Job::Job (SIMinput& simulator, DataHandler& hndler) :
  sim(simulator), handler(hndler)
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
    definePart,                         // definePart(
    nullptr,                            // partInfo()
    nullptr,                            // getNodes()
    nullptr,                            // getElems()
    nullptr,                            // moveNodes()
    nullptr,                            // getPointValues()
    nullptr,                            // getLineNodeValues()
    nullptr,                            // getLineElemValues()
    getFaceNodeValues,                  // getFaceNodeValues()
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


Job::~Job ()
{
  if (mpcciJob)
    mpcci_quit(&mpcciJob);
}

int Job::definePart (MPCCI_SERVER* server, MPCCI_PART* part)
{
  MeshInfo& info = globalInstance->meshInfo;
  info = globalInstance->meshData(part->name);

   MPCCI_MSG_INFO1("Coupling grid definition for component \"%s\" ...\n",
                   MPCCI_PART_NAME(part));

   int ret = smpcci_defp(server,
                         MPCCI_PART_MESHID(part),
                         MPCCI_PART_PARTID(part),
                         MPCCI_PART_CSYS  (part),
                         MPCCI_PART_NNODES(part),
                         MPCCI_PART_NELEMS(part),
                         MPCCI_PART_NAME  (part));

   // Send the nodes definition for this coupled component
   ret = smpcci_pnod(server,
                     MPCCI_PART_MESHID(part),  /* mesh id */
                     MPCCI_PART_PARTID(part),  /* part id */
                     MPCCI_PART_CSYS  (part),  /* coordinates system */
                     MPCCI_PART_NNODES(part),  /* no. of nodes */
                     info.coords.data(),               /* node coordinates */
                     static_cast<unsigned>(sizeof(double)),  /* data type of coordinates */
                     info.nodes.data(),              /* node ids */
                     nullptr);

   // Send the element definiton for this coupled component
   ret = smpcci_pels(server,
                     MPCCI_PART_MESHID(part),    /* mesh id */
                     MPCCI_PART_PARTID(part),    /* part id */
                     MPCCI_PART_NELEMS(part),    /* number of elements */
                     info.types[0],               /* first element type */
                     info.types.data(),                  /* element types */
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

  globalInstance->handler.writeData(MPCCI_QUANT_QID(quant),
                                    globalInstance->meshInfo.nodes,
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

  globalInstance->handler.getData(MPCCI_QUANT_QID(quant),
                                  globalInstance->meshInfo.nodes,
                                  static_cast<double*>(values));
  MPCCI_MSG_INFO0("finished receive values...\n");
}


MeshInfo Job::meshData(std::string_view name) const
{
  const auto& props = globalInstance->sim.getEntity(std::string(name));
  MeshInfo result;
  for (const auto& item : props) {
    std::vector<int> locNodes;
    globalInstance->sim.getTopItemNodes(item, locNodes);
    const ASMbase* pch = globalInstance->sim.getPatch(item.patch);
    for (const int node : locNodes) {
      size_t pnode = pch->getNodeIndex(node,true);
      const Vec3 c = pch->getCoord(pnode);
      result.coords.push_back(c.x);
      result.coords.push_back(c.y);
      result.coords.push_back(c.z);
    }
    IntVec locElms;
    pch->getBoundaryElms(item.item, 0, locElms);
    static const std::vector<std::array<int,4>> eNodes {
        {0, 2, 6, 4},
        {1, 3, 7, 5},
        {0, 1, 5, 4},
        {2, 3, 7, 6},
        {1, 3, 7, 6},
        {2, 3, 7, 6}
    };
    for (const auto& elm : locElms) {
      const auto& elmNodes = pch->getElementNodes(elm+1);
      for (const auto& idx : eNodes[item.item-1])
        result.elms.push_back(elmNodes[idx]);
    }
    for (int& n : locNodes)
      --n;
    result.nodes.insert(result.nodes.end(), locNodes.begin(), locNodes.end());
  }

  result.types.resize(result.elms.size() / 4, MPCCI_ETYP_QUAD4);
  return result;
}

}
