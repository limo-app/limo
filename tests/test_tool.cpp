#include "../src/core/tool.h"
#include <catch2/catch_test_macros.hpp>


TEST_CASE("Overwrite commands are generated", "[.tool]")
{
  const std::string command_string = "my command string";
  Tool tool("t", "", command_string);
  REQUIRE(tool.getRuntime() == Tool::native);
  REQUIRE(tool.getCommand(false) == command_string);
  REQUIRE(tool.getCommand(true) == "flatpak-spawn --host " + command_string);
}

TEST_CASE("Native commands are generated", "[.tool]")
{
  Tool tool_1("t1", "", "prog", "", {}, "");
  REQUIRE(tool_1.getCommand(false) == R"("prog")");
  REQUIRE(tool_1.getRuntime() == Tool::native);
  Tool tool_2("t2", "", "/bin/prog", "/bin", {}, "");
  REQUIRE(tool_2.getCommand(false) == R"(cd "/bin"; "/bin/prog")");
  Tool tool_3("t3", "", "/bin/prog", "/tmp", {{"VAR_1", "VAL_1"}, {"VAR_2", "VAL_2"}}, "");
  REQUIRE(tool_3.getCommand(false) == R"(cd "/tmp"; VAR_1="VAL_1" VAR_2="VAL_2" "/bin/prog")");
  Tool tool_4("t4", "", "/bin/prog", "/tmp", {{"VAR_1", "VAL_1"}, {"VAR_2", "VAL_2"}}, "-v -a 2");
  REQUIRE(tool_4.getCommand(false) == R"(cd "/tmp"; VAR_1="VAL_1" VAR_2="VAL_2" "/bin/prog" -v -a 2)");
  REQUIRE(tool_4.getCommand(true) == R"(flatpak-spawn --host --directory="/tmp" --env=VAR_1="VAL_1" --env=VAR_2="VAL_2" "/bin/prog" -v -a 2)");
}

TEST_CASE("Wine commands are generated", "[.tool]")
{
  Tool tool_1("t1", "", "/bin/prog.exe", "", "", {}, "");
  REQUIRE(tool_1.getCommand(false) == R"(wine "/bin/prog.exe")");
  REQUIRE(tool_1.getRuntime() == Tool::wine);
  Tool tool_2("t2", "", "/bin/prog.exe", "/tmp/wine_prefix", "/tmp", {{"VAR_1", "VAL_1"}}, "-b");
  REQUIRE(tool_2.getCommand(false) == R"(cd "/tmp"; VAR_1="VAL_1" WINEPREFIX="/tmp/wine_prefix" wine "/bin/prog.exe" -b)");
  REQUIRE(tool_2.getCommand(true) == R"(flatpak-spawn --host --directory="/tmp" --env=VAR_1="VAL_1" --env=WINEPREFIX="/tmp/wine_prefix" wine "/bin/prog.exe" -b)");
}

TEST_CASE("Protontricks commands are generated", "[.tool]")
{
  Tool tool_1("t1", "", "/bin/prog.exe", false, 220, "/tmp", {{"VAR_1", "VAL_1"}}, "-arg", "-parg");
  REQUIRE(tool_1.getCommand(false) == R"(cd "/tmp"; VAR_1="VAL_1" protontricks-launch --appid 220 -parg "/bin/prog.exe" -arg)");
  REQUIRE(tool_1.getRuntime() == Tool::protontricks);
  Tool tool_2("t2", "", "/bin/prog.exe", true, 220, "/tmp", {{"VAR_1", "VAL_1"}}, "-arg", "-parg");
  REQUIRE(tool_2.getCommand(true) == R"(flatpak-spawn --host --directory="/tmp" --env=VAR_1="VAL_1" )"
    R"(flatpak run --command=protontricks-launch com.github.Matoking.protontricks --appid 220 -parg "/bin/prog.exe" -arg)");
}

TEST_CASE("Steam commands are generated", "[.tool]")
{
  Tool tool_1("t1", "", 220, false);
  REQUIRE(tool_1.getCommand(false) == "steam -applaunch 220");
  REQUIRE(tool_1.getRuntime() == Tool::steam);
  Tool tool_2("t2", "", 220, true);
  REQUIRE(tool_2.getCommand(true) == "flatpak-spawn --host flatpak run com.valvesoftware.Steam -applaunch 220");
}

TEST_CASE("State is serialized", "[.tool]")
{
  std::vector<Tool> tools =
  {
    {"t1", "", "command string"},
    {"t2", "", "/bin/prog", "/tmp", {{"VAR_1", "VAL_1"}, {"VAR_2", "VAL_2"}}, "-v -a 2"},
    {"t3", "", "/bin/prog.exe", "/tmp/wine_prefix", "/tmp", {{"VAR_1", "VAL_1"}}, "-b"},
    {"t4", "", "/bin/prog.exe", true, 220, "/tmp", {{"VAR_1", "VAL_1"}}, "-arg", "-parg"},
    {"t5", "", 220, true}
  };
  for(const auto& t : tools)
  {
    Tool t2(t.toJson());
    REQUIRE(t.getCommand(false) == t2.getCommand(false));
    REQUIRE(t.getName() == t2.getName());
    REQUIRE(t.getIconPath() == t2.getIconPath());
    REQUIRE(t.getRuntime() == t2.getRuntime());
  }
}
