// $Id$
//==============================================================================
//!
//! \file MpCCIPressureLoad.h
//!
//! \date Mar 15 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Pressure function for MpCCI provided per-face pressures.
//!
//==============================================================================

#ifndef MPCCI_PRESSURE_LOAD_
#define MPCCI_PRESSURE_LOAD_

#include "Function.h"
#include <vector>

class ASMbase;

namespace MpCCI {

struct MeshInfo;

//! \brief Function for scalar pressure load fed from MpCCI.
class PressureLoad : public RealFunc
{
public:
  //! \brief Constructor.
  //! \param info Mesh info for surface grid
  //! \param values Reference to vector of pressure values
  PressureLoad(const std::vector<ASMbase*>& pch,
               const MeshInfo& info, const std::vector<double>& values);

  //! \brief Evaluates the pressure in a point.
  Real evaluate(const Vec3& x) const override;

  //! \brief Sets the active patch.
  bool initPatch(size_t pid) override;

private:
  //! \brief Determines which domain boundary point is on.
  int getDirection(const double* u) const;

  const std::vector<ASMbase*>& m_patches; //!< Reference to underlying patch
  const MeshInfo& m_info; //!< Reference to mesh info for surface grid
  const std::vector<double>& m_values; //!< Reference to vector of pressure values
  std::vector<std::vector<Real>> m_domain; //!< Parameter domain
  size_t m_pid{};
};

}

#endif
