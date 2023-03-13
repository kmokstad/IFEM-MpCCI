// $Id$
//==============================================================================
//!
//! \file SIMMpCCIStructure.h
//!
//! \date Mar 13 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Elastic structure solver for IFEM MpCCI.
//!
//==============================================================================

#ifndef _SIM_MPCCI_STRUCTURE_H_
#define _SIM_MPCCI_STRUCTURE_H_

#include "SIMElasticityWrap.h"


/*!
  \brief Driver wrapping the elasticity solver with a solveStep.
*/

template<class Dim> class SIMMpCCIStructure : public SIMElasticityWrap<Dim>
{
public:
  typedef bool SetupProps; //!< Dummy declaration (no setup properties required)

  //! \brief Default constructor.
  SIMMpCCIStructure();

  //! \brief The destructor clears the VTF-file pointer.
  virtual ~SIMMpCCIStructure() = default;

  //! \brief Computes the solution for the current time step.
  bool solveStep(TimeStep& tp) override;

  //! \brief Returns the actual integrand.
  Elasticity* getIntegrand() override;
};

#endif
