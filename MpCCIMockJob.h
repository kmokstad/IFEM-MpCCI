// $Id$
//==============================================================================
//!
//! \file MpCCIMockJob.h
//!
//! \date Feb 23 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief The IFEM MpCCI adapter mock job class.
//!
//==============================================================================

#ifndef MPCCI_MOCKJOB_H_
#define MPCCI_MOCKJOB_H_

#include "MpCCIDataHandler.h"
#include "MpCCIMeshData.h"

#include <memory>

class HDF5Restart;
class MeshInfo;
class SIMinput;
struct TimeDomain;

namespace MpCCI {

//! \brief Class mocking a MpCCI job.
class MockJob
{
public:
  //! \brief The constructor initializes the MpCCI mock job.
  MockJob(ISerialize& simulator, const double,
          DataHandler* = nullptr,
          GlobalHandler* = nullptr);

  //! \brief Set the input name and creates the coupling.
  void setInputFile(std::string_view name,
                    std::string_view couplingSet,
                    const SIMinput& isim);

  //! \brief Execute data transfer.
  int transfer(int status, TimeDomain& time);

  //! \brief We are done.
  void done() {}

private:
  ISerialize& sim; //!< Reference to IFEM simulator
  int m_level = 1; //!< Current level to read
  MeshInfo m_info; //!< Mesh information
  std::unique_ptr<HDF5Restart> m_reader; //!< Serialized data reader
};

}

#endif
