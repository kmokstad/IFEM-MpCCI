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
#include "MpCCIPressureLoad.h"
#include "SIM3D.h"
#include "SIMMpCCIStructure.h"
#include "SIMsolution.h"

#include "gtest/gtest.h"

#include <numeric>


TEST(TestMpCCIJob, MeshData)
{
  MpCCI::Job::dryRun = true;
  MpCCI::SIMStructure<SIM3D> sim;
  constexpr auto input = R"(
  <geometry dim="3" sets="true">
    <refine patch="1" u="1" v="1" w="1"/>
  </geometry>
  )";

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

  EXPECT_EQ(info1.type, MPCCI_ETYP_QUAD4);

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
  sim.writeData(MPCCI_QID_NPOSITION, info1, displ.data());
  for (size_t i = 0; i < info1.nodes.size(); ++i)
    for (size_t j = 0; j < 3; ++j)
      EXPECT_EQ(displ[3*i+j], 3*info1.nodes[i]+j);

  std::iota(displ.begin(), displ.end(), 0);
  sim.readData(MPCCI_QID_WALLFORCE, info1, displ.data());
  int idx = 0;
  for (const auto& [node, frc] : sim.getLoads()) {
    EXPECT_EQ(node, info1.nodes[idx]);
    EXPECT_EQ(frc[0], idx*3);
    EXPECT_EQ(frc[1], idx*3+1);
    EXPECT_EQ(frc[2], idx*3+2);
    ++idx;
  }
}


TEST(TestMpCCIJob, PressureLoad)
{
  MpCCI::Job::dryRun = true;
  MpCCI::SIMStructure<SIM3D> sim;
  constexpr auto input = R"(
  <geometry dim="3">
    <refine patch="1" u="2" v="2" w="2"/>
    <topologysets>
      <set name="Test" type="face">
        <item patch="1">1 2</item>
      </set>
    </topologysets>
  </geometry>
  )";
  sim.loadXML(input);

  if (!sim.preprocess())
    return;

  if (!sim.initSystem(sim.opt.solver,1))
    return;

  sim.initSolution(sim.getNoDOFs());

  MpCCI::Job job(sim, sim);

  const auto info1 = job.meshData("Test");
  std::vector<double> values(info1.gelms.size());
  for (size_t i = 0; i < info1.gelms.size(); ++i)
    values[i] = info1.gelms[i].first;

  MpCCI::PressureLoad load(sim.getFEModel(), info1, values);

  Vec4 X0(0.0, 0.5, 0.5, 0.0);
  X0.u = X0.ptr();

  double val = load.evaluate(X0);
  EXPECT_EQ(val, 12.0);

  Vec4 X1(1.0, 0.5, 0.5, 0.0);
  X1.u = X1.ptr();
  val = load.evaluate(X1);
  EXPECT_EQ(val, 14.0);
}
