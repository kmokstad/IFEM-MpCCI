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

#include "MpCCIPressureLoad.h"

#include "MpCCIJob.h"

#include "ASMs3D.h"

namespace MpCCI {

PressureLoad::PressureLoad (const std::vector<ASMbase*>& patches,
                            const MeshInfo& info,
                            const std::vector<double>& values) :
  m_patches(patches), m_info(info), m_values(values)
{
    this->initPatch(1);
}


bool PressureLoad::initPatch (size_t pid)
{
    m_pid = pid;
    const ASMs3D& pch = static_cast<const ASMs3D&>(*m_patches[m_pid-1]);
    pch.getParameterDomain(m_domain, nullptr);

    return true;
}


Real PressureLoad::evaluate (const Vec3& X) const
{
    const Vec4* X4 = dynamic_cast<const Vec4*>(&X);
    if (!X4)
      return 0.0;

    const ASMs3D& pch = static_cast<const ASMs3D&>(*m_patches[m_pid-1]);
    int iel = pch.findElementContaining(X4->u);
    int dir = this->getDirection(X4->u);

    const auto it = std::find(m_info.gelms.begin(),
                              m_info.gelms.end(),
                              std::make_pair(iel-1,dir));
    if (it == m_info.gelms.end())
      return 0.0;

    // Negate to get external normal oriented value
    return -m_values[it-m_info.gelms.begin()];
}


int PressureLoad::getDirection (const double* u) const
{
  if (u[0] == m_domain[0][0])
    return 1;
  else if (u[0] == m_domain[0][1])
    return 2;
  else if (u[1] == m_domain[1][0])
    return 3;
  else if (u[1] == m_domain[1][1])
    return 4;
  else if (u[2] == m_domain[2][0])
    return 5;
  else
    return 6;
}

}
