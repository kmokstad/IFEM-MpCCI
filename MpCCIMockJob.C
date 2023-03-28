// $Id$
//==============================================================================
//!
//! \file MpCCIMockJob.C
//!
//! \date Mar 23 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief The IFEM MpCCI adapter mock job class.
//!
//==============================================================================
#include "MpCCIMockJob.h"

#include "MpCCIMeshData.h"
#include "SIMinput.h"

#include <mpcci.h>

namespace MpCCI {

MockJob::MockJob (ISerialize& simulator, const double,
                  DataHandler*, GlobalHandler*) :
  sim(simulator)
{
}


void MockJob::setInputFile(std::string_view name,
                           std::string_view couplingSet,
                           const SIMinput& isim)
{
  m_info = MpCCI::meshData(couplingSet, isim);
  sim.addCoupling(couplingSet, m_info);
  m_reader = std::make_unique<HDF5Restart>(std::string(name),
                                           isim.getProcessAdm());
}


int MockJob::transfer(int status, TimeDomain& time)
{
  HDF5Restart::SerializeData data;
  m_reader->readData(data, m_level++);
  sim.deserializeMpCCIData(data);

  return MPCCI_CONV_STATE_CONTINUE;
}

}
