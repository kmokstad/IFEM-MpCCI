// $Id$
//==============================================================================
//!
//! \file MpCCIArgs.C
//!
//! \date Mar 14 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Preparsing of input files for the MpCCI structure application.
//!
//==============================================================================

#include "MpCCIArgs.h"
#include "Utilities.h"
#include "tinyxml2.h"


bool MpCCIArgs::parse (const tinyxml2::XMLElement* elem)
{
  if (!strcasecmp(elem->Value(),"newmarksolver"))
    dynamic = true;

  if (!strcasecmp(elem->Value(),"elasticity")) {
    std::string formulation;
    if (utl::getAttribute(elem, "formulation", formulation)) {
      if (formulation == "linear")
        form = Formulation::Linear;
      else if (formulation == "TL")
        form = Formulation::TotalLagrangian;
      else if (formulation == "UL")
        form = Formulation::UpdatedLagrangian;
    }
  }

  return this->SIMargsBase::parse(elem);
}
