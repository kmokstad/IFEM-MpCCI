// $Id$
//==============================================================================
//!
//! \file MpCCIDataHandler.h
//!
//! \date Feb 23 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief The data exchange handler for the IFEM MpCCI job class.
//!
//==============================================================================

#ifndef MPCCI_DATA_HANDLER_H_
#define MPCCI_DATA_HANDLER_H_

#include "HDF5Restart.h"

#include <string_view>
#include <vector>

namespace MpCCI {

struct MeshInfo;

//! \brief Abstract data handler for a MpCCI job.
class DataHandler {
public:
  //! \brief Read data from MpCCI server.
  virtual void readData(int quant_id, const MeshInfo& info, const double* data) = 0;

  //! \brief Write data to MpCCI server.
  virtual void writeData(int quand_it, const MeshInfo& info, double* data) const = 0;

  //! \brief Adds the application-specific coupling definition.
  virtual bool addCoupling(std::string_view name, const MeshInfo& info) = 0;
};

class GlobalHandler {
public:
  //! \brief Read global data from MpCCI server (time step size etc).
  virtual void readGlobal(int quant_id, const double* data) = 0;

  //! \brief Write global data to MpCCI server (time step size etc).
  virtual void writeGlobal(int quant_id, double* data) const = 0;

};

//! \brief Virtual interface for MpCCI data (de)serialization.
//! \details Used for mocking a MpCCI server.
class ISerialize : public DataHandler {
public:
  virtual void serializeMpCCIData(HDF5Restart::SerializeData& data) const = 0;
  virtual void deserializeMpCCIData(const HDF5Restart::SerializeData& data) = 0;
};

}

#endif
