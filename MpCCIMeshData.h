// $Id$
//==============================================================================
//!
//! \file MpCCIMeshData.h
//!
//! \date Feb 23 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Utility functions for IFEM<->MpCCI mesh data establishment.
//!
//==============================================================================

#ifndef MPCCI_MESHDATA_H_
#define MPCCI_MESHDATA_H_

#include <string_view>
#include <utility>
#include <vector>

class SIMinput;

namespace MpCCI {

//! \brief Struct for holding a linear FEM mesh definition.
struct MeshInfo {
  std::vector<int> nodes; //!< Global nodes on interface
  std::vector<double> coords; //!< Coordinates for nodes on interface
  std::vector<int> elms; //!< Element node indices on interface
  std::vector<std::pair<int,int>> gelms; //!< Global element numbers for surface
  unsigned type; //!< Type of elements
  int node_per_elm; //!< Nodes per element
};
std::ostream& operator<<(std::ostream&, const MeshInfo&);

MeshInfo meshData(std::string_view name, const SIMinput& sim);

}

#endif
