#include "test_utils.h"
#include "../src/core/bg3deployer.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <ranges>
#include <algorithm>

namespace str = std::ranges;


void resetBg3Files()
{
  const sfs::path source = DATA_DIR / "source" / "bg3" / "target";
  const sfs::path target = DATA_DIR / "target" / "bg3" / "target";
  if(sfs::exists(target))
    sfs::remove_all(target);
  sfs::copy(source, target);
}

TEST_CASE("Plugins are found", "[bg3]")
{
  resetBg3Files();
  
  Bg3Deployer depl(DATA_DIR / "source" / "bg3" / "source", DATA_DIR / "target" / "bg3" / "target", "");
  REQUIRE(depl.getNumMods() == 4);
  const std::vector<std::string> mod_names{"Mod 1", "Mod 2", "Mod 3", "Mod 4"};
  REQUIRE_THAT(depl.getModNames(), 
               Catch::Matchers::UnorderedEquals(mod_names));
  for(const auto& [i, name] : str::enumerate_view(mod_names))
  {
    auto loadorder = depl.getModNames();
    auto iter = str::find(loadorder, name);
    if(iter != loadorder.end())
      depl.swapChild(iter - loadorder.begin(), i);
  }
  REQUIRE_THAT(depl.getModNames(), 
               Catch::Matchers::Equals(mod_names));
  verifyFilesAreEqual(DATA_DIR / "target" / "bg3" / "target" / "modsettings.lsx",
                      DATA_DIR / "target" / "bg3" / "1" / "modsettings.lsx");
}

TEST_CASE("Loadorder can be modified", "[bg3]")
{
  resetBg3Files();
  
  Bg3Deployer depl(DATA_DIR / "source" / "bg3" / "source", DATA_DIR / "target" / "bg3" / "target", "");
  const std::vector<std::string> mod_names{"Mod 1", "Mod 2", "Mod 3", "Mod 4"};
  for(const auto& [i, name] : str::enumerate_view(mod_names))
  {
    auto loadorder = depl.getModNames();
    auto iter = str::find(loadorder, name);
    if(iter != loadorder.end())
      depl.swapChild(iter - loadorder.begin(), i);
  }
  depl.swapChild(3, 0);
  depl.swapChild(2, 1);
  depl.setModStatus(2, false);

  verifyFilesAreEqual(DATA_DIR / "target" / "bg3" / "target" / "modsettings.lsx",
                      DATA_DIR / "target" / "bg3" / "2" / "modsettings.lsx");
  
  Bg3Deployer depl_2(DATA_DIR / "source" / "bg3" / "source", DATA_DIR / "target" / "bg3" / "target", "");
  REQUIRE_THAT(depl.getModNames(), Catch::Matchers::Equals(depl_2.getModNames()));
}

TEST_CASE("Profiles are managed", "[bg3]")
{
  resetBg3Files();
  
  Bg3Deployer depl(DATA_DIR / "source" / "bg3" / "source", DATA_DIR / "target" / "bg3" / "target", "");
  depl.addProfile(-1);

  const std::vector<std::string> mod_names{"Mod 1", "Mod 2", "Mod 3", "Mod 4"};
  for(const auto& [i, name] : str::enumerate_view(mod_names))
  {
    auto loadorder = depl.getModNames();
    auto iter = str::find(loadorder, name);
    if(iter != loadorder.end())
      depl.swapChild(iter - loadorder.begin(), i);
  }
  
  depl.addProfile(0);
  depl.setProfile(1);
  depl.swapChild(3, 0);
  depl.swapChild(2, 1);
  depl.setModStatus(2, false);
  verifyFilesAreEqual(DATA_DIR / "target" / "bg3" / "target" / "modsettings.lsx",
                      DATA_DIR / "target" / "bg3" / "2" / "modsettings.lsx");
  depl.setProfile(0);
  verifyFilesAreEqual(DATA_DIR / "target" / "bg3" / "target" / "modsettings.lsx",
                      DATA_DIR / "target" / "bg3" / "1" / "modsettings.lsx");
}
