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

#include <iosfwd>
#include <string_view>
#include <vector>

class SIMinput;
class TimeStep;

namespace MpCCI {

//! \brief Struct for holding a linear FEM mesh definition.
struct MeshInfo {
  std::vector<int> nodes; //!< Global nodes on interface
  std::vector<double> coords; //!< Coordinates for nodes on interface
  std::vector<int> elms; //!< Element node indices on interface
  std::vector<std::pair<int,int>> gelms; //!< Global element numbers for surface
  unsigned type; //!< Type of elements
};
std::ostream& operator<<(std::ostream&, const MeshInfo&);

//! \brief Class handling a MpCCI job.
//! \details Only a single instance is allowed.
class Job
{
public:
  static Job* globalInstance; //!< Singleton static pointer
  static bool dryRun; //!< To perform a dry run - used in tests

  //! \brief The constructor initializes the MpCCI job.
  Job(SIMinput& simulator, DataHandler& extractor);

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

  //! \brief Extracts data from solution.
  static void extractData(const MeshInfo& info,
                          const std::vector<double>& data,
                          double* values);

  //! \brief Obtain time stepping information.
  const TimeStep& getNextTimeStep();

  //! \brief Execute data transfer.
  int transfer(int status);

  //! \brief Returns the linear FEM mesh for a given topology set.
  MeshInfo meshData(std::string_view name) const;

private:
  MPCCI_JOB* mpcciJob{nullptr}; //!< MpCCI job info structure
  MPCCI_TINFO mpcciTinfo{0}; //!< MpCCI time step information
  MeshInfo meshInfo; //!< Coupling mesh info
  SIMinput& sim; //!< Reference to IFEM simulator
  DataHandler& handler; //!< Data handler
};

}

#endif
