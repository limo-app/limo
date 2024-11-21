#include "../src/core/deployerfactory.h"
#include "../src/core/installer.h"
#include "../src/core/moddedapplication.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <iostream>
#include <ranges>


const int INSTALLER_FLAGS = Installer::preserve_case | Installer::preserve_directories;

TEST_CASE("Mods are installed", "[.app]")
{
  resetStagingDir();
  ModdedApplication app(DATA_DIR / "staging", "test");
  AddModInfo info{
    "mod 0",         "1.0", Installer::SIMPLEINSTALLER, DATA_DIR / "source" / "mod0.tar.gz", {}, -1,
    INSTALLER_FLAGS, 0
  };
  app.installMod(info);
  verifyDirsAreEqual(DATA_DIR / "staging" / "0", DATA_DIR / "source" / "0");
  info.name = "mod 2";
  info.source_path = DATA_DIR / "source" / "mod2.tar.gz";
  app.installMod(info);
  verifyDirsAreEqual(DATA_DIR / "staging" / "1", DATA_DIR / "source" / "2");
  info.name = "mod 1";
  info.source_path = DATA_DIR / "source" / "mod1.zip";
  app.installMod(info);
  verifyDirsAreEqual(DATA_DIR / "staging" / "2", DATA_DIR / "source" / "1");

  info.name = "mod 0->2";
  info.source_path = DATA_DIR / "source" / "mod2.tar.gz";
  info.group = 0;
  info.replace_mod = true;
  app.installMod(info);
  verifyDirsAreEqual(DATA_DIR / "staging" / "0", DATA_DIR / "source" / "2");
  auto mod_info = app.getModInfo();
  REQUIRE(mod_info.size() == 3);
  REQUIRE(mod_info[0].mod.name == "mod 0->2");
}

TEST_CASE("Deployers are added", "[.app]")
{
  resetStagingDir();
  resetAppDir();
  ModdedApplication app(DATA_DIR / "staging", "test");
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER, "depl0", DATA_DIR / "app", Deployer::hard_link });
  AddModInfo info{ "mod 0",
                   "1.0",
                   Installer::SIMPLEINSTALLER,
                   DATA_DIR / "source" / "mod0.tar.gz",
                   { 0 },
                   -1,
                   INSTALLER_FLAGS,
                   0 };
  app.installMod(info);
  info.name = "mod 1";
  info.source_path = DATA_DIR / "source" / "mod1.zip";
  app.installMod(info);
  info.name = "mod 2";
  info.source_path = DATA_DIR / "source" / "mod2.tar.gz";
  app.installMod(info);
  app.deployMods();
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "mod012", true);
}

TEST_CASE("State is saved", "[.app]")
{
  resetStagingDir();
  resetAppDir();
  ModdedApplication app(DATA_DIR / "staging", "test");
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER, "depl0", DATA_DIR / "app", Deployer::hard_link });
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER, "depl1", DATA_DIR / "app_2", Deployer::hard_link });
  app.addProfile(EditProfileInfo{ "test profile", "", -1 });
  app.addTool({"t1", "", "command string"});
  app.addTool({"t4", "", "/bin/prog.exe", true, 220, "/tmp", {{"VAR_1", "VAL_1"}}, "-arg", "-parg"});
  AddModInfo info{ "mod 0",
                   "1.0",
                   Installer::SIMPLEINSTALLER,
                   DATA_DIR / "source" / "mod0.tar.gz",
                   { 0 },
                   -1,
                   INSTALLER_FLAGS,
                   0 };
  app.installMod(info);
  info.name = "mod 1";
  info.source_path = DATA_DIR / "source" / "mod1.zip";
  app.installMod(info);
  info.name = "mod 2";
  info.source_path = DATA_DIR / "source" / "mod2.tar.gz";
  info.deployers = { 0, 1 };
  app.installMod(info);

  ModdedApplication app2(DATA_DIR / "staging", "test2");
  REQUIRE_THAT(app.getDeployerNames(), Catch::Matchers::Equals(app2.getDeployerNames()));
  REQUIRE_THAT(app.getProfileNames(), Catch::Matchers::Equals(app2.getProfileNames()));
  REQUIRE_THAT(app.getLoadorder(0), Catch::Matchers::Equals(app2.getLoadorder(0)));
  auto app_tools = app.getAppInfo().tools;
  auto app2_tools = app2.getAppInfo().tools;
  REQUIRE(app_tools.size() == app2_tools.size());
  for(const auto& [tool_1, tool_2] : std::views::zip(app_tools, app2_tools))
  {
    REQUIRE(tool_1.getName() == tool_2.getName());
    REQUIRE(tool_1.getCommand(false) == tool_2.getCommand(false));
  }
  sfs::create_directories(DATA_DIR / "app_2");
  app2.deployMods();
  verifyDirsAreEqual(DATA_DIR / "app", DATA_DIR / "target" / "mod012", true);
  verifyDirsAreEqual(DATA_DIR / "app_2", DATA_DIR / "source" / "2", true);
  sfs::remove_all(DATA_DIR / "app_2");
}

TEST_CASE("Groups update loadorders", "[.app]")
{
  resetStagingDir();
  ModdedApplication app(DATA_DIR / "staging", "test");
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER, "depl0", DATA_DIR / "app", Deployer::hard_link });
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER, "depl1", DATA_DIR / "app_2", Deployer::hard_link });
  AddModInfo info{ "mod 0",
                   "1.0",
                   Installer::SIMPLEINSTALLER,
                   DATA_DIR / "source" / "mod0.tar.gz",
                   { 0 },
                   -1,
                   INSTALLER_FLAGS,
                   0 };
  app.installMod(info);
  info.name = "mod 1";
  info.deployers = { 0, 1 };
  info.source_path = DATA_DIR / "source" / "mod1.zip";
  app.installMod(info);
  app.createGroup(1, 0);
  REQUIRE_THAT(app.getLoadorder(0), Catch::Matchers::Equals(app.getLoadorder(1)));
  REQUIRE_THAT(app.getLoadorder(0),
               Catch::Matchers::Equals(std::vector<std::tuple<int, bool>>{ { 1, true } }));
  app.changeActiveGroupMember(0, 0);
  REQUIRE_THAT(app.getLoadorder(0),
               Catch::Matchers::Equals(std::vector<std::tuple<int, bool>>{ { 0, true } }));
  info.name = "mod 2";
  info.source_path = DATA_DIR / "source" / "mod2.tar.gz";
  app.installMod(info);
  REQUIRE_THAT(
    app.getLoadorder(0),
    Catch::Matchers::Equals(std::vector<std::tuple<int, bool>>{ { 0, true }, { 2, true } }));
  app.addModToGroup(2, 0);
  REQUIRE_THAT(app.getLoadorder(0),
               Catch::Matchers::Equals(std::vector<std::tuple<int, bool>>{ { 2, true } }));
}

TEST_CASE("Mods are split", "[.app]")
{
  resetStagingDir();
  ModdedApplication app(DATA_DIR / "staging", "test");
  app.addDeployer(
    { DeployerFactory::SIMPLEDEPLOYER, "depl0", DATA_DIR / "source" / "split" / "targets", Deployer::hard_link });
  app.addDeployer({ DeployerFactory::CASEMATCHINGDEPLOYER,
                    "depl1",
                    DATA_DIR / "source" / "split" / "targets" / "a",
                    Deployer::hard_link });
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER,
                    "depl3",
                    DATA_DIR / "source" / "split" / "targets" / "a" / "b",
                    Deployer::hard_link });
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER,
                    "depl3",
                    DATA_DIR / "source" / "split" / "targets" / "a" / "b" / "123",
                    Deployer::hard_link });
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER,
                    "depl4",
                    DATA_DIR / "source" / "split" / "targets" / "a" / "c",
                    Deployer::hard_link });
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER,
                    "depl2",
                    DATA_DIR / "source" / "split" / "targets" / "d",
                    Deployer::hard_link });
  AddModInfo info{ "mod 0",
                   "1.0",
                   Installer::SIMPLEINSTALLER,
                   DATA_DIR / "source" / "split" / "mod",
                   { 0 },
                   -1,
                   INSTALLER_FLAGS,
                   0 };
  app.installMod(info);
  sfs::remove(DATA_DIR / "staging" / "lmm_mods.json");
  sfs::remove(DATA_DIR / "staging" / ".lmm_mods.json.bak");
  verifyDirsAreEqual(DATA_DIR / "staging", DATA_DIR / "target" / "split");
}

TEST_CASE("Mods are uninstalled", "[.app]")
{
  resetStagingDir();
  ModdedApplication app(DATA_DIR / "staging", "test");
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER, "depl0", DATA_DIR / "app", Deployer::hard_link });
  app.addDeployer({ DeployerFactory::SIMPLEDEPLOYER, "depl1", DATA_DIR / "app_2", Deployer::hard_link });
  AddModInfo info{ "mod 0",
                   "1.0",
                   Installer::SIMPLEINSTALLER,
                   DATA_DIR / "source" / "mod0.tar.gz",
                   { 0 },
                   -1,
                   INSTALLER_FLAGS,
                   0 };
  app.installMod(info);
  info.name = "mod 1";
  info.source_path = DATA_DIR / "source" / "mod1.zip";
  app.installMod(info);
  info.name = "mod 2";
  info.source_path = DATA_DIR / "source" / "mod2.tar.gz";
  info.deployers = { 0, 1 };
  info.group = 1;
  app.installMod(info);
  app.uninstallMods({ 0, 2 });
  auto mod_info = app.getModInfo();
  REQUIRE(mod_info.size() == 1);
  REQUIRE(mod_info[0].mod.id == 1);
  REQUIRE(mod_info[0].mod.name == "mod 1");
  REQUIRE_THAT(mod_info[0].deployer_ids, Catch::Matchers::Equals(std::vector<int>{ 0, 1 }));
  REQUIRE_THAT(app.getLoadorder(0),
               Catch::Matchers::Equals(std::vector<std::tuple<int, bool>>{ { 1, true } }));
  REQUIRE_THAT(app.getLoadorder(1),
               Catch::Matchers::Equals(std::vector<std::tuple<int, bool>>{ { 1, true } }));
  verifyDirsAreEqual(DATA_DIR / "staging", DATA_DIR / "target" / "remove" / "simple");

  info.deployers = { 0 };
  info.group = -1;
  info.name = "mod 0";
  info.source_path = DATA_DIR / "source" / "mod0.tar.gz";
  app.installMod(info);
  info.name = "mod 2";
  info.source_path = DATA_DIR / "source" / "mod2.tar.gz";
  info.group = 1;
  app.installMod(info);
  app.uninstallGroupMembers({ 1 });
  mod_info = app.getModInfo();
  REQUIRE(mod_info.size() == 2);
  REQUIRE(mod_info[0].mod.id == 1);
  REQUIRE(mod_info[0].mod.name == "mod 1");
  REQUIRE(mod_info[0].group == -1);
  REQUIRE(mod_info[1].mod.id == 2);
  REQUIRE(mod_info[1].mod.name == "mod 0");
  REQUIRE_THAT(
    app.getLoadorder(0),
    Catch::Matchers::Equals(std::vector<std::tuple<int, bool>>{ { 1, true }, { 2, true } }));
  REQUIRE_THAT(app.getLoadorder(1),
               Catch::Matchers::Equals(std::vector<std::tuple<int, bool>>{ { 1, true } }));
  verifyDirsAreEqual(DATA_DIR / "staging", DATA_DIR / "target" / "remove" / "version");
}
