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

#include "MpCCIDataHandler.h"
#include "SIMElasticityWrap.h"

class IntegrandBase;


namespace MpCCI {

struct MeshInfo;

/*!
  \brief Driver wrapping the elasticity solver with MpCCI data transfer functions.
*/

template<class Dim> class SIMStructure : public SIMElasticityWrap<Dim>,
                                         public MpCCI::DataHandler
{
public:
  //! \brief Default constructor.
  explicit SIMStructure();

  //! \brief Empty destructor.
  virtual ~SIMStructure() = default;

  //! \brief Computes the solution for the current time step.
  bool solveStep(TimeStep& tp) override;

  //! \brief Returns the actual integrand.
  Elasticity* getIntegrand() override;

  //! \brief Get data from MpCCI server.
  void readData(int quant_id, const MeshInfo& info, const double* data) override;

  //! \brief Write data to MpCCI server.
  void writeData(int quant_id, const MeshInfo& info, double* data) const override;

  //! \brief Adds the pressure load function.
  bool addCoupling(std::string_view name, const MeshInfo& info) override;

  //! \brief Returns a const reference to configured loads.
  const std::map<int, Vec3>& getLoads() const { return loadMap; }

protected:
  //! \brief Assemble the nodal interface forces from the fluid solver.
  bool assembleDiscreteTerms(const IntegrandBase*, const TimeDomain&) override;

  std::map<int, Vec3> loadMap; //!< Map of boundary forces
  std::vector<double> elemPressures; //!< Element pressure values
};

}

#endif
