#include "../src/core/deployer.h"
#include "../src/core/reversedeployer.h"
#include "../src/core/pathutils.h"
#include "matcher.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <iostream>

namespace sfs = std::filesystem;


void resetDirs()
{
  if(sfs::exists(DATA_DIR / "source" / "revdepl" / "source"))
    sfs::remove_all(DATA_DIR / "source" / "revdepl" / "source");
  sfs::create_directories(DATA_DIR / "source" / "revdepl" / "source");
  
  if(sfs::exists(DATA_DIR / "target" / "revdepl" / "target"))
  {
    sfs::remove_all(DATA_DIR / "target" / "revdepl" / "target");
    sfs::copy(DATA_DIR / "target" / "revdepl" / "data", DATA_DIR / "target" / "revdepl" / "target",
              sfs::copy_options::recursive);
  }
}

std::vector<std::string> files_to_be_ignored = {
  "some file.txt",
  (sfs::path("a") / "some file.txt").string(),
  (sfs::path("a") / "a" / "some file.txt").string(),
  (sfs::path("b") / "1").string(),
  (sfs::path("b") / "2").string(),
  (sfs::path("c") / "3").string()
};

TEST_CASE("Ignored files are updated", "[revdepl]")
{
  resetDirs();
  ReverseDeployer depl(DATA_DIR / "source" / "revdepl" / "source",
                       DATA_DIR / "target" / "revdepl" / "target",
                       "depl",
                       Deployer::hard_link,
                       false,
                       true);
  depl.addProfile();
  REQUIRE_THAT(depl.getIgnoredFiles(),
               Catch::Matchers::UnorderedEquals(files_to_be_ignored));
  ReverseDeployer depl_2(DATA_DIR / "source" / "revdepl" / "source",
                         DATA_DIR / "target" / "revdepl" / "target",
                         "depl",
                         Deployer::hard_link,
                         false,
                         true);
  REQUIRE_THAT(depl_2.getIgnoredFiles(),
               Catch::Matchers::UnorderedEquals(files_to_be_ignored));
}

TEST_CASE("Managed files are updated", "[revdepl]")
{
  // No extra files
  resetDirs();
  ReverseDeployer depl(DATA_DIR / "source" / "revdepl" / "source",
                       DATA_DIR / "target" / "revdepl" / "target",
                       "depl",
                       Deployer::hard_link,
                       false,
                       true);
  depl.addProfile();
  REQUIRE(depl.getModNames().empty());
  
  // Some extra files with an ignore list
  sfs::copy(DATA_DIR / "target" / "revdepl" / "extra_files",
            DATA_DIR / "target" / "revdepl" / "target",
            sfs::copy_options::skip_existing | sfs::copy_options::recursive);
  depl.updateManagedFiles(true);
  std::vector<std::string> managed_target;
  for(const auto& dir_entry : 
      sfs::recursive_directory_iterator(DATA_DIR / "target" / "revdepl" / "extra_files"))
  {
    if(!dir_entry.is_directory())
      managed_target.push_back(path_utils::getRelativePath(
        dir_entry.path(), DATA_DIR / "target" / "revdepl" / "extra_files"));
  }
  REQUIRE_THAT(depl.getModNames(),
               Catch::Matchers::UnorderedEquals(managed_target));
}

TEST_CASE("Deployed files are ignored", "[revdepl]")
{
  resetDirs();
  std::cout << "Resetting directories..." << std::endl;
  Deployer depl(DATA_DIR / "source" / "revdepl" / "data",
                DATA_DIR / "target" / "revdepl" / "target",
                "depl");
  std::cout << "Adding profile..." << std::endl;
  depl.addProfile();
  std::cout << "Adding mod..." << std::endl;
  depl.addMod(0);
  depl.deploy();
  std::cout << "Files deployed." << std::endl;
  
  std::vector<std::string> managed_target;
  for(const auto& dir_entry : 
      sfs::recursive_directory_iterator(DATA_DIR / "target" / "revdepl" / "extra_files"))
  {
    if(!dir_entry.is_directory())
      managed_target.push_back(path_utils::getRelativePath(
        dir_entry.path(), DATA_DIR / "target" / "revdepl" / "extra_files"));
  }
  
  ReverseDeployer rev_depl(DATA_DIR / "source" / "revdepl" / "source",
                           DATA_DIR / "target" / "revdepl" / "target",
                           "depl",
                           Deployer::hard_link,
                           false,
                           true);
  rev_depl.addProfile();
  sfs::copy(DATA_DIR / "target" / "revdepl" / "extra_files",
            DATA_DIR / "target" / "revdepl" / "target",
            sfs::copy_options::skip_existing | sfs::copy_options::recursive);
  rev_depl.updateManagedFiles();
  REQUIRE_THAT(rev_depl.getModNames(),
               Catch::Matchers::UnorderedEquals(managed_target));
  REQUIRE_THAT(rev_depl.getIgnoredFiles(),
               Catch::Matchers::UnorderedEquals(files_to_be_ignored));
  
  resetDirs();
  depl.deploy();
  Deployer depl_2(DATA_DIR / "source" / "revdepl" / "data",
                  DATA_DIR / "target" / "revdepl" / "target" / "a",
                  "depl");
  depl_2.addProfile();
  depl_2.addMod(1);
  sfs::remove(DATA_DIR / "target" / "revdepl" / "target" / "a" / "some_file.txt");
  depl_2.deploy();
  rev_depl.updateIgnoredFiles(true);
  sfs::copy(DATA_DIR / "target" / "revdepl" / "extra_files",
            DATA_DIR / "target" / "revdepl" / "target",
            sfs::copy_options::skip_existing | sfs::copy_options::recursive);
  rev_depl.updateManagedFiles(true);
  managed_target.erase(std::ranges::find(
    managed_target, (sfs::path("a") / "a" / "1").string()));
  REQUIRE_THAT(rev_depl.getModNames(),
               Catch::Matchers::UnorderedEquals(managed_target));
  std::vector<std::string> new_ignored_target = {
    "some file.txt",
    (sfs::path("b") / "1").string(),
    (sfs::path("b") / "2").string(),
    (sfs::path("c") / "3").string()
  };
  REQUIRE_THAT(rev_depl.getIgnoredFiles(),
               Catch::Matchers::UnorderedEquals(new_ignored_target));
}

TEST_CASE("Managed files are deployed", "[revdepl]")
{
  resetDirs();
  Deployer depl(DATA_DIR / "source" / "revdepl" / "data",
                DATA_DIR / "target" / "revdepl" / "target",
                "depl");
  depl.addProfile();
  depl.addMod(0);
  depl.deploy();
  
  std::vector<std::string> managed_target;
  for(const auto& dir_entry : 
      sfs::recursive_directory_iterator(DATA_DIR / "target" / "revdepl" / "extra_files"))
  {
    if(!dir_entry.is_directory())
      managed_target.push_back(path_utils::getRelativePath(
        dir_entry.path(), DATA_DIR / "target" / "revdepl" / "extra_files"));
  }
  
  ReverseDeployer rev_depl(DATA_DIR / "source" / "revdepl" / "source",
                           DATA_DIR / "target" / "revdepl" / "target",
                           "depl",
                           Deployer::hard_link,
                           false,
                           true);
  rev_depl.addProfile();
  sfs::copy(DATA_DIR / "target" / "revdepl" / "extra_files",
            DATA_DIR / "target" / "revdepl" / "target",
            sfs::copy_options::skip_existing | sfs::copy_options::recursive);
  rev_depl.updateManagedFiles(true);
  
  verifyDirsAreEqual(DATA_DIR / "target" / "revdepl" / "target",
                     DATA_DIR / "target" / "revdepl" / "managed_0", false);
  int id = 0;
  for(const auto& mod : rev_depl.getModNames())
  {
    if(mod == "a/1")
      break;
    id++;
  }
  rev_depl.setModStatus(id, false);
  rev_depl.deploy();
  verifyDirsAreEqual(DATA_DIR / "target" / "revdepl" / "target",
                     DATA_DIR / "target" / "revdepl" / "managed_1", false);
}
