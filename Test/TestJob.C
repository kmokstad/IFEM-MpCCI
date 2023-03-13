//==============================================================================
//!
//! \file TestMpCCIJob.C
//!
//! \date Mar 13 2023
//!
//! \author Arne Morten Kvarving / SINTEF
//!
//! \brief Tests for MpCCI job class.
//!
//==============================================================================

#include "MpCCIJob.h"
#include "SIM3D.h"
#include "SIMMpCCIStructure.h"
#include "SIMsolution.h"

#include "gtest/gtest.h"

constexpr auto input = R"(
<geometry dim="3" sets="true">
  <refine patch="1" u="1" v="1" w="1"/>
</geometry>
)";


TEST(TestMpCCIJob, MeshData)
{
  MpCCI::Job::dryRun = true;
  SIMMpCCIStructure<SIM3D> sim;
  sim.loadXML(input);

  if (!sim.preprocess())
    return;

  if (!sim.initSystem(sim.opt.solver,1))
    return;

  sim.initSolution(sim.getNoDOFs());

  RealArray displacement(sim.getNoDOFs());
  double val = 0.0;
  for (double& d : displacement)
    d = val++;
  sim.setSolution(displacement);

  MpCCI::Job job(sim, sim);

  const auto info1 = job.meshData("Face1");

  EXPECT_EQ(info1.types.size(), 4u);
  for (const auto& t : info1.types)
    EXPECT_EQ(t, MPCCI_ETYP_QUAD4);
  EXPECT_EQ(info1.elms.size(), 4*info1.types.size());

  static const std::vector<int> nodes1 {
    0, 3, 6, 9, 12, 15, 18, 21, 24
  };

  EXPECT_EQ(info1.nodes, nodes1);

  static const std::vector<int> elms1 {
    0, 3, 12, 9,
    3, 6, 15, 12,
    9, 12, 21, 18,
    12, 15, 24, 21
  };

  EXPECT_EQ(info1.elms, elms1);

  static const std::vector<double> coords1 {
    0.0, 0.0, 0.0,
    0.0, 0.5, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 0.5,
    0.0, 0.5, 0.5,
    0.0, 1.0, 0.5,
    0.0, 0.0, 1.0,
    0.0, 0.5, 1.0,
    0.0, 1.0, 1.0
  };

  EXPECT_EQ(info1.coords, coords1);

  std::vector<double> displ(info1.nodes.size()*3);
  sim.writeData(MPCCI_QID_NPOSITION, info1.nodes, displ.data());
  for (size_t i = 0; i < info1.nodes.size(); ++i)
    for (size_t j = 0; j < 3; ++j)
      EXPECT_EQ(displ[3*i+j], 3*info1.nodes[i]+j);
}
