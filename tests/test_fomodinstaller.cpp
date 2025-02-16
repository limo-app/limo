#include "../src/core/fomod/fomodinstaller.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Required files are detected", "[fomod]")
{
  fomod::FomodInstaller installer;
  installer.init(DATA_DIR / "source" / "fomod" / "fomod" / "simple.xml");
  REQUIRE(installer.hasNoSteps());
  auto files = installer.getInstallationFiles({});
  REQUIRE(files.size() == 2);
  REQUIRE(files[0].first == "example.plugin");
  std::vector<std::pair<std::filesystem::path, std::filesystem::path>> target = {
    { "example.plugin", "example.plugin" }, { "another_example.plugin", "another_example.plugin" }
  };
  REQUIRE_THAT(files, Catch::Matchers::Equals(target));
}

TEST_CASE("Steps are executed", "[fomod][!shouldfail]")
{
  fomod::FomodInstaller installer;
  installer.init(DATA_DIR / "source" / "fomod" / "fomod" / "steps.xml");
  REQUIRE(!installer.hasNoSteps());
  auto step = installer.step();
  REQUIRE(step);
  REQUIRE(step->groups.size() == 1);
  REQUIRE(step->groups[0].plugins.size() == 2);
  REQUIRE(step->groups[0].plugins[0].name == "Option A");
  REQUIRE(step->groups[0].plugins[1].name == "Option B");
  REQUIRE(installer.hasNextStep({ { false, true } }));
  step = installer.step({ { false, true } });
  REQUIRE(step);
  REQUIRE(step->groups[0].name == "Select a texture:");
  REQUIRE(step->groups[0].type == fomod::PluginGroup::exactly_one);
  REQUIRE(step->groups[0].plugins[0].name == "Texture Blue");
  REQUIRE(!installer.hasNextStep({ { true, false } }));
  step = installer.step({ { false, true } });
  REQUIRE(!step);
  auto result = installer.getInstallationFiles();
  REQUIRE(result.size() == 2);
  std::vector<std::pair<std::filesystem::path, std::filesystem::path>> target = {
    { "option_b", "option_b" }, { "texture_red_b", "texture_red_b" }
  };
  REQUIRE_THAT(result, Catch::Matchers::Equals(target));
}

TEST_CASE("Installation matrix is parsed", "[fomod][!shouldfail]")
{
  fomod::FomodInstaller installer;
  installer.init(DATA_DIR / "source" / "fomod" / "fomod" / "matrix.xml");
  installer.step();
  installer.step({ { false, true }, { false, true } });
  auto result = installer.getInstallationFiles();
  REQUIRE(result.size() == 2);
  std::vector<std::pair<std::filesystem::path, std::filesystem::path>> target = {
    { "option_b", "option_b" }, { "texture_red_b", "texture_red_b" }
  };
  REQUIRE_THAT(result, Catch::Matchers::UnorderedEquals(target));
}
