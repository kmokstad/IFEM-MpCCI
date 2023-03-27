// $Id$
//==============================================================================
//!
//! \file MpCCIMeshData.C
//!
//! \date Feb 23 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Utility functions for IFEM<->MpCCI mesh data establishment.
//!
//==============================================================================
#include "MpCCIMeshData.h"

#include "ASMbase.h"
#include "IFEM.h"
#include "SIMinput.h"

#include <mpcci.h>

#include <cstdlib>
#include <stdexcept>

namespace {


MpCCI::MeshInfo establish1(std::string_view name, const SIMinput& sim)
{
  const auto& props = sim.getEntity(std::string(name));
  MpCCI::MeshInfo result;
  std::set<int> nodes;
  for (const auto& item : props) {
    std::vector<int> locNodes;
    sim.getTopItemNodes(item, locNodes);
    const ASMbase* pch = sim.getPatch(item.patch);
    IntVec locElms;
    pch->getBoundaryElms(item.item, 0, locElms);
    static const std::vector<std::array<int,4>> eNodes {
        {0, 2, 6, 4},
        {1, 3, 7, 5},
        {0, 1, 5, 4},
        {2, 3, 7, 6},
        {0, 1, 3, 2},
        {4, 5, 7, 6}
    };
    for (const auto& elm : locElms) {
      const auto& elmNodes = pch->getElementNodes(elm+1);
      for (const auto& idx : eNodes[item.item-1])
        result.elms.push_back(elmNodes[idx]);
      result.gelms.emplace_back(elm, item.item);
    }
    for (int& n : locNodes)
      nodes.insert(n-1);
  }
  result.nodes.reserve(nodes.size());
  std::copy(nodes.begin(), nodes.end(), std::back_inserter(result.nodes));
  result.coords.reserve(3*result.nodes.size());
  for (const int node : result.nodes) {
    const Vec3 c = sim.getNodeCoord(node+1);
    result.coords.push_back(c.x);
    result.coords.push_back(c.y);
    result.coords.push_back(c.z);
  }

  result.type = MPCCI_ETYP_QUAD4;
  result.node_per_elm = 4;
  IFEM::cout << result;
  return result;
}


MpCCI::MeshInfo establish2(std::string_view name, const SIMinput& sim)
{
  const auto& props = sim.getEntity(std::string(name));
  MpCCI::MeshInfo result;
  std::set<int> nodes;
  for (const auto& item : props) {
    std::vector<int> locNodes;
    sim.getTopItemNodes(item, locNodes);
    const ASMbase* pch = sim.getPatch(item.patch);
    IntVec locElms;
    pch->getBoundaryElms(item.item, 0, locElms);
    static const std::vector<std::array<int,9>> eNodes {
        {0, 6, 24, 18, 3, 15, 21, 9, 12},
        {2, 8, 26, 20, 5, 17, 23, 11, 14},
        {0, 2, 20, 18, 1, 11, 19, 9, 10},
        {6, 8, 26, 24, 7, 17, 25, 15, 16},
        {0, 2, 8, 6, 1, 5, 7, 3, 4},
        {18, 20, 26, 24, 19, 23, 25, 21, 22}
    };
    for (const auto& elm : locElms) {
      const auto& elmNodes = pch->getElementNodes(elm+1);
      for (const auto& idx : eNodes[item.item-1])
        result.elms.push_back(elmNodes[idx]);
      result.gelms.emplace_back(elm, item.item);
    }
    for (int& n : locNodes)
      nodes.insert(n-1);
  }
  result.nodes.reserve(nodes.size());
  std::copy(nodes.begin(), nodes.end(), std::back_inserter(result.nodes));
  result.coords.reserve(3*result.nodes.size());
  for (const int node : result.nodes) {
    const Vec3 c = sim.getNodeCoord(node+1);
    result.coords.push_back(c.x);
    result.coords.push_back(c.y);
    result.coords.push_back(c.z);
  }

  result.type = MPCCI_ETYP_QUAD9;
  result.node_per_elm = 9;
  IFEM::cout << result;
  return result;
}


}


namespace MpCCI {


MeshInfo meshData(std::string_view name, const SIMinput& sim)
{
  int n1,n2,n3;
  sim.getFEModel()[0]->getOrder(n1,n2,n3);
  if (n1 == 2 && n2 == 2 && n3 == 2)
    return establish1(name, sim);
  else if (n1 == 3 && n2 == 3 && n3 == 3) {
    if (sim.opt.discretization != ASM::Lagrange)
      throw std::runtime_error("Quadratic elements only supported for lagrange.");
    return establish2(name, sim);
  } else
    throw std::runtime_error("Unsupported element order");
}


std::ostream& operator<<(std::ostream& os, const MeshInfo& info)
{
  os << "MeshInfo: nnod = " << info.nodes.size()
     << " nelms = " << info.elms.size() / info.node_per_elm;
  os << "\n== Nodes ==";
  auto cit = info.coords.begin();
  for (size_t i = 0; i < info.nodes.size(); ++i) {
    os << "\n\t" << info.nodes[i] << ": ";
    for (size_t j = 0; j < 3; ++j)
      os << *cit++ << " ";
  }
  auto it = info.elms.begin();
  os << "\n== Elements ==";
  for (size_t i = 0; i < info.elms.size() / info.node_per_elm; ++i) {
    os << "\n\t" << i << " -> (" << info.gelms[i].first << "," << info.gelms[i].second <<"): ";
    for (int j = 0; j < info.node_per_elm; ++j)
      os << *it++ << " ";
  }
  os << std::endl;
  return os;
}


}
