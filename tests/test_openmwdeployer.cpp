#include "../src/core/openmwarchivedeployer.h"
#include "../src/core/openmwplugindeployer.h"
#include "matcher.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <ranges>
#include <algorithm>

namespace str = std::ranges;


void resetOpenMwFiles()
{
  const sfs::path source_source = DATA_DIR / "source" / "openmw" / "source";
  const sfs::path source_target = DATA_DIR / "target" / "openmw" / "source";
  const sfs::path target_source = DATA_DIR / "source" / "openmw" / "target";
  const sfs::path target_target = DATA_DIR / "target" / "openmw" / "target";
  if(sfs::exists(source_target))
    sfs::remove_all(source_target);
  if(sfs::exists(target_target))
    sfs::remove_all(target_target);
  sfs::copy(source_source, source_target);
  sfs::copy(target_source, target_target);
}

TEST_CASE("State is read", "[openmw]")
{
  auto expectedEntry0 = std::make_shared<DeployerModInfo>(false, "mod 0", "", -1, true);
  auto expectedEntry1 = std::make_shared<DeployerModInfo>(false, "mod 1", "", -1, true);
  auto expectedEntry2 = std::make_shared<DeployerModInfo>(false, "mod 2", "", -1, true);
  auto expectedEntry3 = std::make_shared<DeployerModInfo>(false, "mod 3", "", -1, true);
  auto expectedEntry4 = std::make_shared<DeployerModInfo>(false, "mod 4", "", -1, true);
  auto expectedEntry5 = std::make_shared<DeployerModInfo>(false, "mod 5", "", -1, true);
  auto expectedEntry6 = std::make_shared<DeployerModInfo>(false, "mod 6", "", -1, true);
  auto expectedEntry7 = std::make_shared<DeployerModInfo>(false, "mod 7", "", -1, true);
  std::vector<std::weak_ptr<DeployerEntry>> expectedEntries0 = {
    expectedEntry0,
    expectedEntry1,
    expectedEntry2,
  };
  std::vector<std::weak_ptr<DeployerEntry>> expectedEntries1 = {
    expectedEntry0,
    expectedEntry1,
    expectedEntry2,
    expectedEntry3,
    expectedEntry4,
    expectedEntry5,
    expectedEntry6,
    expectedEntry7,
  };

  resetOpenMwFiles();
  
  OpenMwArchiveDeployer a_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  REQUIRE(a_depl.getNumMods() == 3);
  REQUIRE_THAT(a_depl.getModNames(),
               Catch::Matchers::Equals(std::vector<std::string>{ "Morrowind.bsa", "b.bsa", "a.bsa" }));
  REQUIRE_THAT(a_depl.getLoadorder()->getTraversalItems(),
              EqualsDeployerEntryVector(expectedEntries0));
  
  OpenMwPluginDeployer p_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  const std::vector<std::string> mod_names{"Morrowind.esm", "f.omwgame", "c.esp", "d.EsP", "a.esp", "e.omwaddon",
                                           "g.omwscripts", "h.omwscripts"};
  REQUIRE(p_depl.getNumMods() == mod_names.size());
  for(const auto& [i, name] : str::enumerate_view(mod_names))
  {
    auto loadorder = p_depl.getModNames();
    auto iter = str::find(loadorder, name);
    if(iter != loadorder.end())
      p_depl.swapChild(iter - loadorder.begin(), i);
  }
  REQUIRE_THAT(p_depl.getModNames(),
               Catch::Matchers::Equals(mod_names));
  REQUIRE_THAT(p_depl.getLoadorder()->getTraversalItems(), EqualsDeployerEntryVector(expectedEntries1));
  
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "0" / "openmw.cfg");
}

TEST_CASE("Load order can be edited", "[openmw]")
{
  resetOpenMwFiles();
  
  OpenMwArchiveDeployer a_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  OpenMwPluginDeployer p_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  
  const std::vector<std::string> mod_names{"Morrowind.esm", "f.omwgame", "c.esp", "d.EsP", "a.esp", "e.omwaddon",
                                           "g.omwscripts", "h.omwscripts"};
  for(const auto& [i, name] : str::enumerate_view(mod_names))
  {
    auto loadorder = p_depl.getModNames();
    auto iter = str::find(loadorder, name);
    if(iter != loadorder.end())
      p_depl.swapChild(iter - loadorder.begin(), i);
  }
  
  a_depl.swapChild(1, 2);
  p_depl.swapChild(1, 3);
  p_depl.swapChild(2, 1);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "1" / "openmw.cfg");
  
  a_depl.setModStatus(1, false);
  p_depl.setModStatus(3, false);
  p_depl.applyModAction(0, 2);
  p_depl.applyModAction(0, 4);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "2" / "openmw.cfg");
  
  OpenMwArchiveDeployer a_depl_2(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  OpenMwPluginDeployer p_depl_2(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  
  REQUIRE_THAT(a_depl.getModNames(), Catch::Matchers::Equals(a_depl_2.getModNames()));
  REQUIRE_THAT(p_depl.getModNames(), Catch::Matchers::Equals(p_depl_2.getModNames()));
  REQUIRE_THAT(a_depl.getLoadorder()->getTraversalItems(), EqualsDeployerEntryVector(a_depl_2.getLoadorder()->getTraversalItems()));
  REQUIRE_THAT(p_depl.getLoadorder()->getTraversalItems(), EqualsDeployerEntryVector(p_depl_2.getLoadorder()->getTraversalItems()));
}

TEST_CASE("Profiles are managed", "[openmw]")
{
  resetOpenMwFiles();
  
  OpenMwArchiveDeployer a_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  OpenMwPluginDeployer p_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  
  const std::vector<std::string> mod_names{"Morrowind.esm", "f.omwgame", "c.esp", "d.EsP", "a.esp", "e.omwaddon",
                                           "g.omwscripts", "h.omwscripts"};
  for(const auto& [i, name] : str::enumerate_view(mod_names))
  {
    auto loadorder = p_depl.getModNames();
    auto iter = str::find(loadorder, name);
    if(iter != loadorder.end())
      p_depl.swapChild(iter - loadorder.begin(), i);
  }
  
  a_depl.addProfile(-1);
  p_depl.addProfile(-1);
  a_depl.addProfile(0);
  p_depl.addProfile(0);
  a_depl.setProfile(1);
  p_depl.setProfile(1);
  a_depl.swapChild(1, 2);
  p_depl.swapChild(1, 3);
  p_depl.swapChild(2, 1);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "1" / "openmw.cfg");
  
  a_depl.addProfile(1);
  p_depl.addProfile(1);
  a_depl.setProfile(2);
  p_depl.setProfile(2);
  a_depl.setModStatus(1, false);
  p_depl.setModStatus(3, false);
  p_depl.applyModAction(0, 2);
  p_depl.applyModAction(0, 4);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "2" / "openmw.cfg");
  
  a_depl.setProfile(0);
  p_depl.setProfile(0);
  p_depl.applyModAction(1, 2);
  p_depl.applyModAction(1, 4);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "0" / "openmw.cfg");
  
  a_depl.setProfile(1);
  p_depl.setProfile(1);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "1" / "openmw.cfg");
}
