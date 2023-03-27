// $Id$
//==============================================================================
//!
//! \file MpCCIArgs.h
//!
//! \date Mar 14 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Preparsing of input files for the MpCCI structure application.
//!
//==============================================================================

#ifndef _MPCCI_ARGS_H
#define _MPCCI_ARGS_H

#include "SIMargsBase.h"


class TiXmlElement;


/*!
  \brief Class holding application parameters.
*/

class MpCCIArgs: public SIMargsBase
{
public:
  bool dynamic = false;

  enum class Formulation {
    Linear,
    TotalLagrangian,
    UpdatedLagrangian
  };

  Formulation form = Formulation::Linear;

  //! \brief Default constructor.
  MpCCIArgs() : SIMargsBase("elasticity") {}

protected:
  //! \brief Parse an element from the input file
  bool parse(const TiXmlElement* elem) override;
};

#endif
