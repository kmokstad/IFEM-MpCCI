// $Id$
//==============================================================================
//!
//! \file SIMThermoElasticity.C
//!
//! \date Aug 05 2014
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Wrapper equipping the elasticity solver with temperature coupling.
//!
//==============================================================================

#include "SIMMpCCIStructure.h"

#include "SIMLinEl.h"

#include "Profiler.h"
#include "SIM3D.h"

template<>
bool SIMLinEl<SIM3D>::parseDimSpecific (char* cline)
{
  return false;
}


template<>
bool SIMLinEl<SIM3D>::parseDimSpecific (const TiXmlElement* child,
                                        const std::string& type)
{
  return false;
}


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

template class SIMMpCCIStructure<SIM3D>;
