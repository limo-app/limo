#include "../src/core/openmwarchivedeployer.h"
#include "../src/core/openmwplugindeployer.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>


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

TEST_CASE("State is read", "[.openmw]")
{
  resetOpenMwFiles();
  
  OpenMwArchiveDeployer a_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  REQUIRE(a_depl.getNumMods() == 3);
  REQUIRE_THAT(a_depl.getModNames(),
               Catch::Matchers::Equals(std::vector<std::string>{ "Morrowind.bsa", "b.bsa", "a.bsa" }));
  REQUIRE_THAT(a_depl.getLoadorder(),
               Catch::Matchers::Equals(
                 std::vector<std::tuple<int, bool>>{ { -1, true }, { -1, true }, { -1, true } }));
  
  OpenMwPluginDeployer p_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "", false);
  REQUIRE(p_depl.getNumMods() == 4);
  REQUIRE_THAT(p_depl.getModNames(),
               Catch::Matchers::Equals(std::vector<std::string>{ "Morrowind.esm", "c.esl", "d.EsP", "a.esp" }));
  REQUIRE_THAT(p_depl.getLoadorder(),
               Catch::Matchers::Equals(
                 std::vector<std::tuple<int, bool>>{ { -1, true }, { -1, true }, { -1, true }, { -1, true } }));
  
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "0" / "openmw.cfg");
}

TEST_CASE("Load order can be edited", "[.openmw]")
{
  resetOpenMwFiles();
  
  OpenMwArchiveDeployer a_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  OpenMwPluginDeployer p_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "", false);
  
  a_depl.changeLoadorder(1, 2);
  p_depl.changeLoadorder(1, 3);
  p_depl.changeLoadorder(2, 1);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "1" / "openmw.cfg");
  
  a_depl.setModStatus(1, false);
  p_depl.setModStatus(3, false);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "2" / "openmw.cfg");
  
  OpenMwArchiveDeployer a_depl_2(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  OpenMwPluginDeployer p_depl_2(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "", false);
  
  REQUIRE_THAT(a_depl.getModNames(), Catch::Matchers::Equals(a_depl_2.getModNames()));
  REQUIRE_THAT(p_depl.getModNames(), Catch::Matchers::Equals(p_depl_2.getModNames()));
  REQUIRE_THAT(a_depl.getLoadorder(), Catch::Matchers::Equals(a_depl_2.getLoadorder()));
  REQUIRE_THAT(p_depl.getLoadorder(), Catch::Matchers::Equals(p_depl_2.getLoadorder()));
}

TEST_CASE("Profiles are managed", "[.openmw]")
{
  resetOpenMwFiles();
  
  OpenMwArchiveDeployer a_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "");
  OpenMwPluginDeployer p_depl(
    DATA_DIR / "target" / "openmw" / "source", DATA_DIR / "target" / "openmw" / "target", "", false);
  
  a_depl.addProfile(-1);
  p_depl.addProfile(-1);
  a_depl.addProfile(0);
  p_depl.addProfile(0);
  a_depl.setProfile(1);
  p_depl.setProfile(1);
  a_depl.changeLoadorder(1, 2);
  p_depl.changeLoadorder(1, 3);
  p_depl.changeLoadorder(2, 1);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "1" / "openmw.cfg");
  
  a_depl.addProfile(1);
  p_depl.addProfile(1);
  a_depl.setProfile(2);
  p_depl.setProfile(2);
  a_depl.setModStatus(1, false);
  p_depl.setModStatus(3, false);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "2" / "openmw.cfg");
  
  a_depl.setProfile(0);
  p_depl.setProfile(0);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "0" / "openmw.cfg");
  
  a_depl.setProfile(1);
  p_depl.setProfile(1);
  verifyFilesAreEqual(DATA_DIR / "target" / "openmw" / "target" / "openmw.cfg", DATA_DIR / "target" / "openmw" / "1" / "openmw.cfg");
}
