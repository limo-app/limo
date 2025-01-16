#include "openmwplugindeployer.h"
#include <format>
#include <fstream>
#include <ranges>

namespace sfs = std::filesystem;
namespace str = std::ranges;


OpenMwPluginDeployer::OpenMwPluginDeployer(const sfs::path& source_path,
                               const sfs::path& dest_path,
                               const std::string& name,
                               bool init_tags) :
  LootDeployer(source_path, dest_path, name, init_tags, false)
{
  type_ = "OpenMW Plugin Deployer";
  is_autonomous_ = true;
  plugin_regex_ = R"(.*\.[eE][sS][pPlLmM]$)";
  plugin_file_line_regex_ = R"(^\s*(\*?)([^#]*\.[eE][sS][pPlLmM])(\r?))";
  config_file_name_ = ".plugin_config";
  tags_file_name_ = ".plugin_tags";
  source_mods_file_name_ = ".plugin_mod_sources";
  updateAppType();
  plugin_file_name_ = ".plugins.txt";
  if(!initPluginFile())
    loadPlugins();
  updatePlugins();
  if(sfs::exists(dest_path_ / config_file_name_))
    loadSettings();
  if(init_tags)
    readPluginTags();
  readSourceMods();
}

void OpenMwPluginDeployer::unDeploy(std::optional<ProgressNode*> progress_node)
{
  const std::string plugin_backup_path =
    dest_path_ / ("." + plugin_file_name_ + UNDEPLOY_BACKUP_EXTENSION);
  if(!sfs::exists(plugin_backup_path))
    sfs::copy(dest_path_ / plugin_file_name_, plugin_backup_path);

  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating plugins...", name_));
  updatePlugins();
}

void OpenMwPluginDeployer::addProfile(int source)
{
  PluginDeployer::addProfile(source);
}

void OpenMwPluginDeployer::removeProfile(int profile)
{
  PluginDeployer::removeProfile(profile);
}

void OpenMwPluginDeployer::setProfile(int profile)
{
  PluginDeployer::setProfile(profile);
}

void OpenMwPluginDeployer::writePlugins() const
{
  PluginDeployer::writePlugins();

  const sfs::path plugin_file_path = dest_path_ / OPEN_MW_CONFIG_FILE_NAME;
  std::ifstream in_file(plugin_file_path);
  if(!in_file.is_open())
    throw std::runtime_error(std::format("Error: Could not open '{}'.", plugin_file_path.string()));

  std::vector<std::string> lines;
  int content_line = -1;
  bool found_archive = false;
  std::regex content_regex(R"(^content=.*)");
  std::string line;
  int i = 0;
  while(getline(in_file, line))
  {
    std::smatch match;
    if(std::regex_match(line, match, content_regex))
    {
      if(!found_archive)
        content_line = i;
      found_archive = true;
    }
    else
      lines.push_back(line);
    i++;
  }
  in_file.close();

  std::ofstream out_file(plugin_file_path);
  if(!out_file.is_open())
    throw std::runtime_error(std::format("Error: Could not open '{}'.", plugin_file_path.string()));

  for(const auto& [i, line] : str::enumerate_view(lines))
  {
    if(i == content_line)
    {
      for(const auto& [plugin, enabled] : plugins_)
      {
        if(enabled)
          out_file << "content=" + plugin + "\n";
      }
    }
    out_file << line << "\n";
  }
  if(content_line == -1 || content_line >= lines.size())
  {
    for(const auto& [plugin, enabled] : plugins_)
    {
      if(enabled)
        out_file << "content=" + plugin + "\n";
    }
  }
}

bool OpenMwPluginDeployer::initPluginFile()
{
  const sfs::path plugin_file_path = dest_path_ / plugin_file_name_;
  if(sfs::exists(plugin_file_path))
    return false;;

  const sfs::path config_file_path = dest_path_ / OPEN_MW_CONFIG_FILE_NAME;
  std::ifstream in_file(config_file_path);
  if(!in_file.is_open())
    throw std::runtime_error(std::format("Error: Could not open '{}'.", config_file_path.string()));

  std::string line;
  std::regex plugin_regex(R"(^content=(.*?\.[eE][sS][pPlLmM]))");
  while(getline(in_file, line))
  {
    std::smatch match;
    if(std::regex_match(line, match, plugin_regex))
      plugins_.emplace_back(match[1], true);
  }

  PluginDeployer::writePlugins();
  return true;
}
