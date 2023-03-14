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

#include <vector>

namespace MpCCI {

//! \brief Abstract data handler for a MpCCI job.
class DataHandler {
public:
  //! \brief Read data from MpCCI server.
  virtual void readData(int quant_id, const std::vector<int>& nodes, const double* data) = 0;

  //! \brief Write data to MpCCI server.
  virtual void writeData(int quand_it, const std::vector<int>& nodes, double* data) const = 0;
};

}

#endif
