#include "../src/core/casematchingdeployer.h"
#include "../src/core/deployer.h"
#include "matcher.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <set>
#include <ranges>


TEST_CASE("Mods are added and removed", "[deployer]")
{
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(2, true);
  REQUIRE(depl.getNumMods() == 1);
  depl.removeMod(2);
  REQUIRE(depl.getNumMods() == 0);
}

TEST_CASE("Mods are being deployed", "[deployer]")
{
  resetAppDir();
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(1, true);
  depl.deploy();
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "mod1", true);
}

TEST_CASE("Mod status works", "[deployer]")
{
  resetAppDir();
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(1, false);
  depl.setModStatus(1, true);
  depl.addMod(0, true);
  depl.setModStatus(0, false);
  depl.deploy();
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "mod1", true);
}

TEST_CASE("Deployed mods are removed", "[deployer]")
{
  resetAppDir();
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(1, true);
  depl.deploy();
  depl.setModStatus(1, false);
  depl.deploy();
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "source" / "app", true);
}

TEST_CASE("Conflicts are resolved", "[deployer]")
{
  resetAppDir();
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(0, true);
  depl.addMod(1, true);
  depl.addMod(2, true);
  depl.deploy();
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "mod012", true);
}

TEST_CASE("Files are restored", "[deployer]")
{
  resetAppDir();
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(0, true);
  depl.addMod(1, true);
  depl.addMod(2, true);
  depl.deploy();
  depl.setModStatus(0, false);
  depl.setModStatus(1, false);
  depl.setModStatus(2, false);
  depl.deploy();
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "source" / "app", true);
}

TEST_CASE("Loadorder is being changed", "[deployer]")
{
  resetAppDir();
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(2, true);
  depl.addMod(0, true);
  depl.addMod(1, true);
  depl.swapChild(1, 0);
  depl.swapChild(1, 2);
  depl.deploy();
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "mod012", true);
}

TEST_CASE("Profiles", "[deployer]")
{
  resetAppDir();
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(1, true);
  depl.deploy();
  depl.addProfile(0);
  depl.setProfile(1);
  depl.addMod(0, true);
  depl.addMod(2, true);
  depl.swapChild(0, 1);
  depl.deploy();
  SECTION("Copy profile")
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "mod012", true);
  SECTION("Create new profile")
  {
    depl.addProfile();
    depl.setProfile(2);
    depl.deploy();
    verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "source" / "app", true);
  }
}

TEST_CASE("Get mod conflicts", "[deployer]")
{
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(0, true);
  depl.addMod(1, true);
  depl.addMod(2, true);
  auto conflicts = depl.getModConflicts(1);
  REQUIRE(conflicts.size() == 1);
  REQUIRE(conflicts.contains(1));
  conflicts = depl.getModConflicts(0);
  REQUIRE(conflicts.size() == 2);
  REQUIRE(conflicts.contains(2));
  REQUIRE(conflicts.contains(0));
}

TEST_CASE("Get file conflicts", "[deployer]")
{
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(0, true);
  depl.addMod(1, true);
  depl.addMod(2, true);
  auto conflicts = depl.getFileConflicts(1);
  REQUIRE(conflicts.size() == 0);
  conflicts = depl.getFileConflicts(0);
  REQUIRE(conflicts.size() == 3);
}

TEST_CASE("Conflict groups are created", "[deployer]")
{
  Deployer depl(DATA_DIR / "source" / "conflicts", DATA_DIR / "app", "");
  depl.addProfile();
  for(int i : { 0, 1, 2, 3, 4, 5, 6, 7 })
    depl.addMod(i, true);
  depl.updateConflictGroups();
  auto groups = depl.getConflictGroups();
  REQUIRE_THAT(groups,
               Catch::Matchers::UnorderedEquals(
                 std::vector<std::vector<int>>{ { 0, 1, 2, 3, 5 }, { 4, 6 }, { 7 } }));
}

TEST_CASE("Mods are sorted", "[deployer]")
{
  // Arrange
  auto expectedEntry0 = std::make_shared<DeployerModInfo>(false, "mod 0", "", 0, true);
  auto expectedEntry1 = std::make_shared<DeployerModInfo>(false, "mod 1", "", 1, true);
  auto expectedEntry2 = std::make_shared<DeployerModInfo>(false, "mod 2", "", 2, true);
  auto expectedEntry3 = std::make_shared<DeployerModInfo>(false, "mod 3", "", 3, true);
  auto expectedEntry4 = std::make_shared<DeployerModInfo>(false, "mod 4", "", 4, true);
  auto expectedEntry5 = std::make_shared<DeployerModInfo>(false, "mod 5", "", 5, true);
  auto expectedEntry6 = std::make_shared<DeployerModInfo>(false, "mod 6", "", 6, true);
  auto expectedEntry7 = std::make_shared<DeployerModInfo>(false, "mod 7", "", 7, true);
  std::vector<std::weak_ptr<DeployerEntry>> expectedEntries = {
    expectedEntry5,
    expectedEntry0,
    expectedEntry2,
    expectedEntry1,
    expectedEntry3,
    expectedEntry6,
    expectedEntry4,
    expectedEntry7
  };

  Deployer depl(DATA_DIR / "source" / "conflicts", DATA_DIR / "app", "");
  depl.addProfile();
  for(int i : { 5, 6, 0, 7, 4, 2, 1, 3 })
    depl.addMod(i, true);
  depl.sortModsByConflicts();
  REQUIRE_THAT(depl.getLoadorder()->getTraversalItems(),
               EqualsDeployerEntryVector(expectedEntries));
}

TEST_CASE("Case matching deployer", "[deployer]")
{
  resetAppDir();
  sfs::remove_all(DATA_DIR / "source" / "case_matching" / "0");
  sfs::remove_all(DATA_DIR / "source" / "case_matching" / "1");
  sfs::copy_options options(sfs::copy_options::recursive | sfs::copy_options::overwrite_existing);
  sfs::copy(DATA_DIR / "source" / "case_matching" / "orig_0",
            DATA_DIR / "source" / "case_matching" / "0",
            options);
  sfs::copy(DATA_DIR / "source" / "case_matching" / "orig_1",
            DATA_DIR / "source" / "case_matching" / "1",
            options);
  CaseMatchingDeployer depl(DATA_DIR / "source" / "case_matching", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(0, true);
  depl.addMod(1, true);
  depl.deploy({ 0, 1 });
  verifyDirsAreEqual(DATA_DIR / "source" / "case_matching" / "0",
                     DATA_DIR / "target" / "case_matching" / "0",
                     false);
  verifyDirsAreEqual(DATA_DIR / "source" / "case_matching" / "1",
                     DATA_DIR / "target" / "case_matching" / "1",
                     false);
}

TEST_CASE("External changes are handeld", "[deployer]")
{
  resetAppDir();
  resetStagingDir();
  sfs::copy(DATA_DIR / "source" / "0", DATA_DIR / "staging" / "0", sfs::copy_options::recursive);
  sfs::copy(DATA_DIR / "source" / "1", DATA_DIR / "staging" / "1", sfs::copy_options::recursive);
  sfs::copy(DATA_DIR / "source" / "2", DATA_DIR / "staging" / "2", sfs::copy_options::recursive);
  
  Deployer depl = Deployer(DATA_DIR / "staging", DATA_DIR / "app", "");
  depl.addProfile();
  depl.addMod(0, true);
  depl.addMod(1, true);
  depl.addMod(2, true);
  depl.deploy();
  
  // mod - file - keep
  // 0 - b/3aBc - true
  // 1 - 6 - false
  // 2 - 0.txt - true
  
  sfs::remove(DATA_DIR / "app" / "0.txt");
  sfs::copy(DATA_DIR / "source" / "external_changes" / "0.txt", DATA_DIR / "app");
  sfs::remove(DATA_DIR / "app" / "6");
  sfs::copy(DATA_DIR / "source" / "external_changes" / "6", DATA_DIR / "app");
  sfs::remove(DATA_DIR / "app" / "b" / "3aBc");
  sfs::copy(DATA_DIR / "source" / "external_changes" / "3aBc", DATA_DIR / "app" / "b");
  
  auto detected_changes = depl.getExternallyModifiedFiles();
  std::set<std::pair<std::string, int>> actual_changes = {{"0.txt", 2}, {"6", 1}, {(std::filesystem::path("b") / "3aBc").string(), 0}};
  REQUIRE(detected_changes.size() == actual_changes.size());
  for(const auto& [path, id] : detected_changes)
    REQUIRE(actual_changes.contains({path.string(), id}));
  
  FileChangeChoices changes_to_keep;
  for(const auto& [path, id] : detected_changes)
  {
    changes_to_keep.paths.push_back(path);
    changes_to_keep.mod_ids.push_back(id);
    changes_to_keep.changes_to_keep.push_back(true);
  }
  changes_to_keep.changes_to_keep[1] = false;
  depl.keepOrRevertFileModifications(changes_to_keep);
  
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "external_changes", true);
  REQUIRE(sfs::equivalent(DATA_DIR / "staging" / "0" / "b" / "3aBc", DATA_DIR / "app" / "b" / "3aBc"));
  REQUIRE(sfs::equivalent(DATA_DIR / "staging" / "1" / "6", DATA_DIR / "app" / "6"));
  REQUIRE(sfs::equivalent(DATA_DIR / "staging" / "2" / "0.txt", DATA_DIR / "app" / "0.txt"));
}

TEST_CASE("Files are deployed as sym links", "[deployer]")
{
  resetAppDir();
  Deployer depl = Deployer(DATA_DIR / "source", DATA_DIR / "app", "", Deployer::sym_link);
  depl.addProfile();
  depl.addMod(0, true);
  depl.addMod(1, true);
  depl.addMod(2, true);
  depl.deploy();
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "mod012", false);
  for(const auto& dir_entry : std::filesystem::recursive_directory_iterator(DATA_DIR / "app"))
  {
    // exclude directories, files which are not overwritten and .lmmfiles
    if(!dir_entry.is_directory() && dir_entry.path().extension() != ".lmmbak"
      && dir_entry.path().filename() != ".lmmfiles" && dir_entry.path().filename() != "file.cfg"
      && dir_entry.path().filename() != "wasd" && dir_entry.path().filename() != "0"
      && dir_entry.path().filename() != ".lmm_managed_dir")
        REQUIRE(std::filesystem::is_symlink(dir_entry.path()));
  }
}
