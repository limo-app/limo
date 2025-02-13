#include "../src/core/backupmanager.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <iostream>


TEST_CASE("Backups are created", "[backup]")
{
  resetAppDir();
  BackupManager bak_man;
  bak_man.addProfile();
  bak_man.addTarget(DATA_DIR / "app" / "a", "t", { "b0", "b1" });
  bak_man.addTarget(DATA_DIR / "app" / "0.txt", "t2", { "b0", "b1" });
  verifyFilesAreEqual(DATA_DIR / "app" / "0.txt", DATA_DIR / "app" / "0.txt.1.lmmbakman");
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "create_bak");
  verifyDirsAreEqual(DATA_DIR / "app" / "a", DATA_DIR / "app" / "a.1.lmmbakman", true);
  sfs::remove(DATA_DIR / "app" / "a.1.lmmbakman" / "2.txt");
  bak_man.addBackup(0, "b2");
  verifyDirsAreEqual(DATA_DIR / "app" / "a", DATA_DIR / "app" / "a.2.lmmbakman", true);
  bak_man.addBackup(0, "b3", 1);
  verifyDirsAreEqual(DATA_DIR / "app" / "a.1.lmmbakman", DATA_DIR / "app" / "a.3.lmmbakman", true);
  sfs::remove(DATA_DIR / "app" / "a.1.lmmbakman" / "file.cfg");
  bak_man.addBackup(0, "b4", 0);
  verifyDirsAreEqual(DATA_DIR / "app" / "a", DATA_DIR / "app" / "a.4.lmmbakman", true);
}

TEST_CASE("Backups are activated", "[backup]")
{
  resetAppDir();
  BackupManager bak_man;
  bak_man.addProfile();
  bak_man.addTarget(DATA_DIR / "app" / "a", "t", { "b0", "b1" });
  bak_man.addTarget(DATA_DIR / "app" / "a-Fil _3", "t2", { "b0", "b1" });
  sfs::copy(DATA_DIR / "source" / "bak_man" / "a-Fil _3",
            DATA_DIR / "app",
            sfs::copy_options::overwrite_existing);
  verifyFilesAreEqual(DATA_DIR / "app" / "a-Fil _3",
                      DATA_DIR / "target" / "bak_man" / "change_bak" / "a-Fil _3.0.lmmbakman");
  bak_man.setActiveBackup(1, 1);
  verifyFilesAreEqual(DATA_DIR / "app" / "a-Fil _3",
                      DATA_DIR / "target" / "bak_man" / "change_bak" / "a-Fil _3");
  verifyFilesAreEqual(DATA_DIR / "app" / "a-Fil _3.0.lmmbakman",
                      DATA_DIR / "target" / "bak_man" / "change_bak" / "a-Fil _3.0.lmmbakman");
  sfs::remove(DATA_DIR / "app" / "a" / "2.txt");
  sfs::copy(DATA_DIR / "source" / "bak_man" / "file.cfg",
            DATA_DIR / "app" / "a",
            sfs::copy_options::overwrite_existing);
  bak_man.setActiveBackup(0, 1);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "change_bak");
  verifyDirsAreEqual(DATA_DIR / "app" / "a.0.lmmbakman",
                     DATA_DIR / "target" / "bak_man" / "change_bak" / "a.0.lmmbakman",
                     true);
  bak_man.setActiveBackup(0, 0);
  verifyDirsAreEqual(
    DATA_DIR / "target" / "bak_man" / "change_bak" / "a.0.lmmbakman", DATA_DIR / "app" / "a", true);
  verifyDirsAreEqual(
    DATA_DIR / "target" / "bak_man" / "change_bak" / "a", DATA_DIR / "app" / "a.1.lmmbakman", true);
}

TEST_CASE("Backups are removed", "[backup]")
{
  resetAppDir();
  BackupManager bak_man;
  bak_man.addProfile();
  bak_man.addTarget(DATA_DIR / "app" / "a", "t", { "b0", "b1", "b2", "b3", "b4" });
  sfs::remove(DATA_DIR / "app" / "a.3.lmmbakman" / "2.txt");
  sfs::remove(DATA_DIR / "app" / "a.1.lmmbakman" / "file.cfg");
  sfs::copy(DATA_DIR / "app" / "a.4.lmmbakman" / "2.txt",
            DATA_DIR / "app" / "a.4.lmmbakman" / "newfile");
  bak_man.removeBackup(0, 2);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "remove_bak_0");

  bak_man.removeBackup(0, 0);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "remove_bak_1");
  bak_man.setActiveBackup(0, 1);
  bak_man.removeBackup(0, 1);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "remove_bak_2");
}

TEST_CASE("Profiles are working", "[backup]")
{
  resetAppDir();
  BackupManager bak_man;
  bak_man.addProfile();
  bak_man.addTarget(DATA_DIR / "app" / "a", "t", { "b0", "b1", "b2", "b3", "b4" });
  sfs::remove(DATA_DIR / "app" / "a.3.lmmbakman" / "2.txt");
  sfs::remove(DATA_DIR / "app" / "a.1.lmmbakman" / "file.cfg");
  sfs::copy(DATA_DIR / "app" / "a.4.lmmbakman" / "2.txt",
            DATA_DIR / "app" / "a.4.lmmbakman" / "newfile");
  bak_man.addTarget(DATA_DIR / "app" / "a-Fil _3", "t2", { "b0", "b1" });
  sfs::copy(DATA_DIR / "source" / "bak_man" / "a-Fil _3",
            DATA_DIR / "app",
            sfs::copy_options::overwrite_existing);
  bak_man.setActiveBackup(0, 2);
  bak_man.setActiveBackup(1, 1);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "profiles_0");
  verifyFilesAreEqual(DATA_DIR / "app" / "a-Fil _3",
                      DATA_DIR / "target" / "bak_man" / "profiles_0" / "a-Fil _3");
  bak_man.addProfile(0);
  bak_man.addProfile(-1);
  bak_man.setActiveBackup(0, 1);
  bak_man.setActiveBackup(1, 0);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "profiles_1");
  verifyFilesAreEqual(DATA_DIR / "app" / "a-Fil _3",
                      DATA_DIR / "target" / "bak_man" / "profiles_1" / "a-Fil _3");
  bak_man.setProfile(1);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "profiles_0");
  verifyFilesAreEqual(DATA_DIR / "app" / "a-Fil _3",
                      DATA_DIR / "target" / "bak_man" / "profiles_0" / "a-Fil _3");
  bak_man.setProfile(2);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "profiles_2");
  verifyFilesAreEqual(DATA_DIR / "app" / "a-Fil _3",
                      DATA_DIR / "target" / "bak_man" / "profiles_2" / "a-Fil _3");
  bak_man.removeProfile(2);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "profiles_1");
  verifyFilesAreEqual(DATA_DIR / "app" / "a-Fil _3",
                      DATA_DIR / "target" / "bak_man" / "profiles_1" / "a-Fil _3");
}

TEST_CASE("State is saved", "[backup]")
{
  resetAppDir();
  BackupManager bak_man;
  bak_man.addProfile();
  bak_man.addTarget(DATA_DIR / "app" / "a", "t", { "b0", "b1", "b2", "b3", "b4" });
  sfs::remove(DATA_DIR / "app" / "a.3.lmmbakman" / "2.txt");
  sfs::remove(DATA_DIR / "app" / "a.1.lmmbakman" / "file.cfg");
  sfs::copy(DATA_DIR / "app" / "a.4.lmmbakman" / "2.txt",
            DATA_DIR / "app" / "a.4.lmmbakman" / "newfile");
  bak_man.addTarget(DATA_DIR / "app" / "a-Fil _3", "t2", { "b0", "b1" });
  sfs::copy(DATA_DIR / "source" / "bak_man" / "a-Fil _3",
            DATA_DIR / "app",
            sfs::copy_options::overwrite_existing);
  bak_man.setActiveBackup(0, 2);
  bak_man.setActiveBackup(1, 1);
  bak_man.addProfile(0);
  bak_man.setActiveBackup(0, 1);
  bak_man.setActiveBackup(1, 0);
  const auto targets_orig = bak_man.getTargets();
  BackupManager bak_man2;
  bak_man2.addProfile();
  bak_man2.addProfile();
  bak_man2.addTarget(DATA_DIR / "app" / "a", "t", { "b0", "b1", "b2", "b3", "b4" });
  bak_man2.addTarget(DATA_DIR / "app" / "a-Fil _3", "t2", { "b0", "b1" });
  const auto targets_new = bak_man2.getTargets();
  REQUIRE_THAT(targets_orig, Catch::Matchers::Equals(targets_new));
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "profiles_1");
  verifyFilesAreEqual(DATA_DIR / "app" / "a-Fil _3",
                      DATA_DIR / "target" / "bak_man" / "profiles_1" / "a-Fil _3");
  bak_man2.setProfile(1);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "profiles_0");
  verifyFilesAreEqual(DATA_DIR / "app" / "a-Fil _3",
                      DATA_DIR / "target" / "bak_man" / "profiles_0" / "a-Fil _3");
}

TEST_CASE("Invalid state is repaired", "[backup]")
{
  resetAppDir();
  BackupManager bak_man;
  bak_man.addProfile();
  bak_man.addTarget(DATA_DIR / "app" / "a", "t", { "b0", "b1", "b2", "b3", "b4" });
  sfs::remove(DATA_DIR / "app" / "a.3.lmmbakman" / "2.txt");
  sfs::remove(DATA_DIR / "app" / "a.1.lmmbakman" / "file.cfg");
  sfs::copy(DATA_DIR / "app" / "a.4.lmmbakman" / "2.txt",
            DATA_DIR / "app" / "a.4.lmmbakman" / "newfile");
  bak_man.addTarget(DATA_DIR / "app" / "a-Fil _3", "t2", { "b0", "b1" });
  sfs::remove_all(DATA_DIR / "app" / "a.3.lmmbakman");
  sfs::copy(DATA_DIR / "app" / "a.2.lmmbakman", DATA_DIR / "app" / "a.8.lmmbakman");
  sfs::copy(DATA_DIR / "app" / "a.2.lmmbakman", DATA_DIR / "app" / "a.15.lmmbakman");
  sfs::remove(DATA_DIR / "app" / "a-Fil _3.1.lmmbakman");
  bak_man.setActiveBackup(0, 2);
  bak_man.setActiveBackup(1, 0);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "invalid_state");
}

TEST_CASE("Targets are removed", "[backup]")
{
  resetAppDir();
  BackupManager bak_man;
  bak_man.addProfile();
  bak_man.addTarget(DATA_DIR / "app" / "a", "t", { "b0", "b1", "b2", "b3", "b4" });
  sfs::remove(DATA_DIR / "app" / "a.3.lmmbakman" / "2.txt");
  sfs::remove(DATA_DIR / "app" / "a.1.lmmbakman" / "file.cfg");
  sfs::copy(DATA_DIR / "app" / "a.4.lmmbakman" / "2.txt",
            DATA_DIR / "app" / "a.4.lmmbakman" / "newfile");
  bak_man.addTarget(DATA_DIR / "app" / "a-Fil _3", "t2", { "b0", "b1" });
  bak_man.setActiveBackup(0, 2);
  bak_man.removeTarget(0);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "bak_man" / "remove_target");
  bak_man.removeTarget(0);
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "source" / "app", true);
}

TEST_CASE("Backups are overwritten", "[backup]")
{
  resetAppDir();
  BackupManager bak_man;
  bak_man.addProfile();
  bak_man.addTarget(DATA_DIR / "app" / "a", "t", { "b0", "b1" });
  sfs::remove(DATA_DIR / "app" / "a" / "file.cfg");
  sfs::copy(DATA_DIR / "source" / "bak_man" / "file.cfg",
            DATA_DIR / "app" / "a.1.lmmbakman",
            sfs::copy_options::overwrite_existing);
  sfs::copy(DATA_DIR / "source" / "bak_man" / "a-Fil _3", DATA_DIR / "app" / "a.1.lmmbakman");
  verifyDirsAreEqual(DATA_DIR / "app" / "a", DATA_DIR / "target" / "bak_man" / "overwrite0", true);
  bak_man.overwriteBackup(0, 1, 0);
  verifyDirsAreEqual(DATA_DIR / "app" / "a", DATA_DIR / "target" / "bak_man" / "overwrite1", true);
}
