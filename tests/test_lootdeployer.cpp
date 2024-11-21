#include "../src/core/lootdeployer.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>


void resetFiles()
{
  sfs::path plugin_target = DATA_DIR / "target" / "loot" / "target" / "plugins.txt";
  sfs::path plugin_source = DATA_DIR / "source" / "loot" / "plugins.txt";
  sfs::path load_order_target = DATA_DIR / "target" / "loot" / "target" / "loadorder.txt";
  sfs::path load_order_source = DATA_DIR / "source" / "loot" / "loadorder.txt";
  for(const auto& dir_entry : sfs::directory_iterator(DATA_DIR / "target" / "loot" / "target"))
    sfs::remove(dir_entry.path());
  sfs::copy(plugin_source, plugin_target);
  sfs::copy(load_order_source, load_order_target);
}


TEST_CASE("State is read", "[.loot]")
{
  resetFiles();
  LootDeployer depl(
    DATA_DIR / "target" / "loot" / "source", DATA_DIR / "target" / "loot" / "target", "", false);
  REQUIRE(depl.getNumMods() == 4);
  REQUIRE_THAT(depl.getModNames(),
               Catch::Matchers::Equals(std::vector<std::string>{ "a.esp", "c.esp", "Morrowind.esm", "d.esp" }));
  REQUIRE_THAT(depl.getLoadorder(),
               Catch::Matchers::Equals(
                 std::vector<std::tuple<int, bool>>{ { -1, true }, { -1, false }, { -1, true }, { -1, true } }));
}

TEST_CASE("Load order can be edited", "[.loot]")
{
  resetFiles();
  LootDeployer depl(
    DATA_DIR / "target" / "loot" / "source", DATA_DIR / "target" / "loot" / "target", "", false);
  depl.changeLoadorder(0, 2);
  depl.setModStatus(1, true);
  depl.setModStatus(0, false);
  depl.changeLoadorder(2, 1);
  REQUIRE_THAT(depl.getModNames(),
               Catch::Matchers::Equals(std::vector<std::string>{ "c.esp", "a.esp", "Morrowind.esm", "d.esp" }));
  REQUIRE_THAT(depl.getLoadorder(),
               Catch::Matchers::Equals(
                 std::vector<std::tuple<int, bool>>{ { -1, false }, { -1, true }, { -1, true }, { -1, true } }));
  LootDeployer depl2(
    DATA_DIR / "target" / "loot" / "source", DATA_DIR / "target" / "loot" / "target", "", false);
  REQUIRE_THAT(depl.getModNames(), Catch::Matchers::Equals(depl2.getModNames()));
  REQUIRE_THAT(depl.getLoadorder(), Catch::Matchers::Equals(depl2.getLoadorder()));
}

TEST_CASE("Profiles are managed", "[.loot]")
{
  resetFiles();
  LootDeployer depl(
    DATA_DIR / "target" / "loot" / "source", DATA_DIR / "target" / "loot" / "target", "", false);
  depl.addProfile(5);
  depl.addProfile(0);
  depl.setModStatus(0, false);
  REQUIRE_THAT(depl.getLoadorder(),
               Catch::Matchers::Equals(
                 std::vector<std::tuple<int, bool>>{ { -1, false }, { -1, false }, { -1, true }, { -1, true } }));
  depl.setProfile(1);
  REQUIRE_THAT(depl.getLoadorder(),
               Catch::Matchers::Equals(
                 std::vector<std::tuple<int, bool>>{ { -1, true }, { -1, false }, { -1, true }, { -1, true } }));
  depl.addProfile(0);
  depl.setProfile(2);
  REQUIRE_THAT(depl.getLoadorder(),
               Catch::Matchers::Equals(
                 std::vector<std::tuple<int, bool>>{ { -1, false }, { -1, false }, { -1, true }, { -1, true } }));
  verifyDirsAreEqual(
    DATA_DIR / "target" / "loot" / "target", DATA_DIR / "target" / "loot" / "profiles", true);
}
