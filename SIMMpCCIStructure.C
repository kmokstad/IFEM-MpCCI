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

#include "SIMMpCCIStructure.h"

#include "ElasticityUtils.h"
#include "LinearElasticity.h"

#include "AlgEqSystem.h"
#include "Profiler.h"
#include "SAM.h"
#include "SIM3D.h"

#include <mpcci_quantities.h>


template<class Dim>
SIMMpCCIStructure<Dim>::SIMMpCCIStructure ()
{
  Dim::myHeading = "Structure solver";
}


template<class Dim>
bool SIMMpCCIStructure<Dim>::solveStep (TimeStep& tp)
{
  PROFILE1("SIMMpCCIStructure::solveStep");

  this->setMode(SIM::STATIC);
  this->setQuadratureRule(Dim::opt.nGauss[0]);
  if (!this->assembleSystem())
    return false;

  if (!this->solveSystem(SIMsolution::solution.front(),1))
    return false;

  return true;
}

template<class Dim>
Elasticity* SIMMpCCIStructure<Dim>::getIntegrand ()
{
  if (!Dim::myProblem)
    Dim::myProblem = new LinearElasticity(Dim::dimension,
                                          Elastic::axiSymmetry,
                                          Elastic::GIpointsVTF);

  return dynamic_cast<Elasticity*>(Dim::myProblem);
}

template<class Dim>
bool SIMMpCCIStructure<Dim>::assembleDiscreteTerms (const IntegrandBase* p,
                                                   const TimeDomain&)
{
  if (p != Dim::myProblem)
    return false;

  SystemVector* b = Dim::myEqSys->getVector();

  for (const auto& [node, load] : loadMap) {
    Dim::mySam->assembleSystem(*b, load.ptr(), node);
  }

  return true;
}

template<class Dim>
void SIMMpCCIStructure<Dim>::writeData (int quant_id,
                                      const std::vector<int>& nodes,
                                      double* valptr) const
{
  if (quant_id != MPCCI_QID_NPOSITION)
    throw std::runtime_error("Asked to write an unknown quantity " + std::to_string(quant_id));

  for (const int idx : nodes)
    for (size_t i = 0; i < Dim::dimension; ++i)
      *valptr++ = this->getSolution()[idx*Dim::dimension+i];
}

template<class Dim>
void SIMMpCCIStructure<Dim>::getData (int quant_id,
                                      const std::vector<int>& nodes,
                                      const double* valptr)
{
  if (quant_id != MPCCI_QID_WALLFORCE)
    throw std::runtime_error("Asked to read an unknown quantity " + std::to_string(quant_id));
  loadMap.clear();
  for (int node : nodes) {
    Vec3 frc;
    for (size_t i = 0; i < Dim::dimension; ++i)
      frc[i] = *valptr++;
    loadMap.emplace(node, frc);
  }
}

template class SIMMpCCIStructure<SIM3D>;
