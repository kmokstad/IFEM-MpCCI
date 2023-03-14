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
#include "tinyxml.h"


bool MpCCIArgs::parse (const TiXmlElement* elem)
{
  if (!strcasecmp(elem->Value(),"newmarksolver"))
    newmark = true;

  return this->SIMargsBase::parse(elem);
}
