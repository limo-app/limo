#include "../src/core/lootdeployer.h"
#include "matcher.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>


void resetFiles()
{
  const sfs::path plugin_target = DATA_DIR / "target" / "loot" / "target" / "plugins.txt";
  const sfs::path plugin_source = DATA_DIR / "source" / "loot" / "plugins.txt";
  const sfs::path load_order_target = DATA_DIR / "target" / "loot" / "target" / "loadorder.txt";
  const sfs::path load_order_source = DATA_DIR / "source" / "loot" / "loadorder.txt";
  for(const auto& dir_entry : sfs::directory_iterator(DATA_DIR / "target" / "loot" / "target"))
    sfs::remove(dir_entry.path());
  sfs::copy(plugin_source, plugin_target);
  sfs::copy(load_order_source, load_order_target);
}


TEST_CASE("State is read", "[loot]")
{
  // Arrange
  auto expectedEntry0 = std::make_shared<DeployerModInfo>(false, "mod 0", "", -1, true);
  auto expectedEntry1 = std::make_shared<DeployerModInfo>(false, "mod 1", "", -1, false);
  auto expectedEntry2 = std::make_shared<DeployerModInfo>(false, "mod 2", "", -1, true);
  auto expectedEntry3 = std::make_shared<DeployerModInfo>(false, "mod 3", "", -1, true);
  std::vector<std::weak_ptr<DeployerEntry>> expectedEntries = {
    expectedEntry0,
    expectedEntry1,
    expectedEntry2,
    expectedEntry3
  };

  resetFiles();
  LootDeployer depl(
    DATA_DIR / "target" / "loot" / "source", DATA_DIR / "target" / "loot" / "target", "", false);
  REQUIRE(depl.getNumMods() == 4);
  REQUIRE_THAT(depl.getModNames(),
               Catch::Matchers::Equals(std::vector<std::string>{ "a.esp", "c.esp", "Morrowind.esm", "d.esp" }));
  REQUIRE_THAT(depl.getLoadorder()->getTraversalItems(),
               EqualsDeployerEntryVector(expectedEntries));
}

TEST_CASE("Load order can be edited", "[loot]")
{
  auto expectedEntry0 = std::make_shared<DeployerModInfo>(false, "mod 0", "", -1, false);
  auto expectedEntry1 = std::make_shared<DeployerModInfo>(false, "mod 1", "", -1, true);
  auto expectedEntry2 = std::make_shared<DeployerModInfo>(false, "mod 2", "", -1, true);
  auto expectedEntry3 = std::make_shared<DeployerModInfo>(false, "mod 3", "", -1, true);
  std::vector<std::weak_ptr<DeployerEntry>> expectedEntries = {
    expectedEntry0,
    expectedEntry1,
    expectedEntry2,
    expectedEntry3
  };

  resetFiles();
  LootDeployer depl(
    DATA_DIR / "target" / "loot" / "source", DATA_DIR / "target" / "loot" / "target", "", false);
  depl.swapChild(0, 2);
  depl.setModStatus(1, true);
  depl.setModStatus(0, false);
  depl.swapChild(2, 1);
  REQUIRE_THAT(depl.getModNames(),
               Catch::Matchers::Equals(std::vector<std::string>{ "c.esp", "a.esp", "Morrowind.esm", "d.esp" }));
  REQUIRE_THAT(depl.getLoadorder()->getTraversalItems(),
               EqualsDeployerEntryVector(expectedEntries));
  LootDeployer depl2(
    DATA_DIR / "target" / "loot" / "source", DATA_DIR / "target" / "loot" / "target", "", false);
  REQUIRE_THAT(depl.getModNames(), Catch::Matchers::Equals(depl2.getModNames()));
  REQUIRE_THAT(depl.getLoadorder()->getTraversalItems(), EqualsDeployerEntryVector(depl2.getLoadorder()->getTraversalItems()));
}

TEST_CASE("Profiles are managed", "[loot]")
{
  auto expectedEntry0f = std::make_shared<DeployerModInfo>(false, "mod 0", "", -1, false);
  auto expectedEntry0t = std::make_shared<DeployerModInfo>(false, "mod 0", "", -1, true);
  auto expectedEntry1 = std::make_shared<DeployerModInfo>(false, "mod 1", "", -1, false);
  auto expectedEntry2 = std::make_shared<DeployerModInfo>(false, "mod 2", "", -1, true);
  auto expectedEntry3 = std::make_shared<DeployerModInfo>(false, "mod 3", "", -1, true);
  std::vector<std::weak_ptr<DeployerEntry>> expectedEntries0 = {
    expectedEntry0f,
    expectedEntry1,
    expectedEntry2,
    expectedEntry3
  };
  std::vector<std::weak_ptr<DeployerEntry>> expectedEntries1 = {
    expectedEntry0t,
    expectedEntry1,
    expectedEntry2,
    expectedEntry3
  };

  resetFiles();
  LootDeployer depl(
    DATA_DIR / "target" / "loot" / "source", DATA_DIR / "target" / "loot" / "target", "", false);
  depl.addProfile(5);
  depl.addProfile(0);
  depl.setModStatus(0, false);
  REQUIRE_THAT(depl.getLoadorder()->getTraversalItems(),
               EqualsDeployerEntryVector(expectedEntries0));
  depl.setProfile(1);
  REQUIRE_THAT(depl.getLoadorder()->getTraversalItems(),
               EqualsDeployerEntryVector(expectedEntries1));
  depl.addProfile(0);
  depl.setProfile(2);
  REQUIRE_THAT(depl.getLoadorder()->getTraversalItems(),
              EqualsDeployerEntryVector(expectedEntries0));
  verifyDirsAreEqual(
    DATA_DIR / "target" / "loot" / "target", DATA_DIR / "target" / "loot" / "profiles", true);
}
