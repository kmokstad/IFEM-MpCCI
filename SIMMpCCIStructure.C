// $Id$
//==============================================================================
//!
//! \file SIMpCCIStructure.C
//!
//! \date Aug 05 2014
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Wrapper equipping the elasticity solver with temperature coupling.
//!
//==============================================================================

#include "MpCCIJob.h"
#include "MpCCIPressureLoad.h"
#include "SIMMpCCIStructure.h"

#include "ElasticityUtils.h"
#include "LinearElasticity.h"

#include "AlgEqSystem.h"
#include "Profiler.h"
#include "SAM.h"
#include "SIM3D.h"
#include "TimeStep.h"
#include "TractionField.h"

#include <mpcci_quantities.h>

namespace MpCCI {

template<class Dim>
SIMStructure<Dim>::SIMStructure ()
{
  Dim::myHeading = "MpCCI Structure solver";
}


template<class Dim>
bool SIMStructure<Dim>::solveStep (TimeStep& tp)
{
  PROFILE1("MpCCI::SIMStructure::solveStep");

  this->setMode(SIM::STATIC);
  this->setQuadratureRule(Dim::opt.nGauss[0]);
  if (!this->assembleSystem())
    return false;

  if (!this->solveSystem(SIMsolution::solution.front(),1))
    return false;

  return true;
}


template<class Dim>
Elasticity* SIMStructure<Dim>::getIntegrand ()
{
  if (!Dim::myProblem)
    Dim::myProblem = new LinearElasticity(Dim::dimension,
                                          Elastic::axiSymmetry,
                                          Elastic::GIpointsVTF);

  return dynamic_cast<Elasticity*>(Dim::myProblem);
}


template<class Dim>
bool SIMStructure<Dim>::assembleDiscreteTerms (const IntegrandBase* p,
                                               const TimeDomain&)
{
  if (p != Dim::myProblem)
    return false;

  SystemVector* b = Dim::myEqSys->getVector();

  for (const auto& [node, load] : loadMap)
    Dim::mySam->assembleSystem(*b, load.ptr(), node);

  return true;
}


template<class Dim>
void SIMStructure<Dim>::writeData (int quant_id,
                                   const MeshInfo& info,
                                   double* valptr) const
{
  if (quant_id != MPCCI_QID_NPOSITION)
    throw std::runtime_error("Asked to write an unknown quantity " +
                             std::to_string(quant_id));

  for (const int idx : info.nodes) {
    const Vec3 pos = this->getNodeCoord(idx+1);
    for (size_t i = 0; i < Dim::dimension; ++i) {
      *valptr++ = this->getSolution()[idx*Dim::dimension+i] + pos[i];
    }
  }
}


template<class Dim>
void SIMStructure<Dim>::readData (int quant_id,
                                  const MeshInfo& info,
                                  const double* valptr)
{
  if (quant_id == MPCCI_QID_WALLFORCE) {
    loadMap.clear();
    for (int node : info.nodes) {
      Vec3 frc;
      std::copy(valptr, valptr + Dim::dimension, &frc[0]);
      valptr += Dim::dimension;
      loadMap.emplace(node, frc);
    }
  } else if (quant_id == MPCCI_QID_ABSPRESSURE) {
    elemPressures.resize(info.gelms.size());
    std::copy(valptr, valptr + info.gelms.size(), elemPressures.begin());
  } else {
    throw std::runtime_error("Asked to read an unknown quantity " +
                             std::to_string(quant_id));
  }
}


template<class Dim>
bool SIMStructure<Dim>::addCoupling (std::string_view name,
                                    const MeshInfo& info)
{
  int code = this->getUniquePropertyCode(std::string(name));
  PressureLoad* load = new PressureLoad(this->getFEModel(), info, elemPressures);
  this->myTracs[code] = new PressureField(load);
  if (!this->setPropertyType(code,Property::NEUMANN))
    return false;
  for (const Property& prop : this->myProps) {
    if (prop.pindx == code)
      this->generateThreadGroups(prop);
  }

  return true;
}

template class SIMStructure<SIM3D>;

}
