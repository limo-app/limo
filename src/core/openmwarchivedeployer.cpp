#include "openmwarchivedeployer.h"
#include <format>
#include <fstream>
#include <ranges>
#include "pathutils.h"

namespace sfs = std::filesystem;
namespace str = std::ranges;
namespace pu = path_utils;


OpenMwArchiveDeployer::OpenMwArchiveDeployer(const std::filesystem::path& source_path,
                                             const std::filesystem::path& dest_path,
                                             const std::string& name) :
  PluginDeployer(source_path, dest_path, name)
{
  type_ = "OpenMW Archive Deployer";
  is_autonomous_ = true;
  plugin_regex_ = R"(.*\.[bB][sS][aA]$)";
  plugin_file_line_regex_ = R"(^\s*(\*?)([^#]*\.[bB][sS][aA])(\r?))";
  plugin_file_name_ = ".archives.txt";
  config_file_name_ = ".archives_config";
  tags_file_name_ = ".archives_tags";
  source_mods_file_name_ = ".archives_mod_sources";
  if(!initPluginFile())
    loadPlugins();
  updatePlugins();
  if(sfs::exists(dest_path_ / config_file_name_))
    loadSettings();
}

void OpenMwArchiveDeployer::unDeploy(std::optional<ProgressNode*> progress_node)
{
  const std::string plugin_backup_path =
    dest_path_ / ("." + plugin_file_name_ + UNDEPLOY_BACKUP_EXTENSION);
  if(!pu::exists(plugin_backup_path))
    sfs::copy(dest_path_ / plugin_file_name_, plugin_backup_path, sfs::copy_options::overwrite_existing);

  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating plugins...", name_));
  updatePlugins();
}

bool OpenMwArchiveDeployer::supportsSorting() const
{
  return false;
}

bool OpenMwArchiveDeployer::supportsModConflicts() const
{
  return false;
}

void OpenMwArchiveDeployer::writePlugins() const
{
  PluginDeployer::writePlugins();

  const sfs::path plugin_file_path = dest_path_ / OPEN_MW_CONFIG_FILE_NAME;
  std::ifstream in_file(plugin_file_path);
  if(!in_file.is_open())
    throw std::runtime_error(std::format("Error: Could not open '{}'.", plugin_file_path.string()));

  std::vector<std::string> lines;
  int archive_line = -1;
  bool found_archive = false;
  std::regex archive_regex(R"(^fallback-archive=.*)");
  std::string line;
  int i = 0;
  while(getline(in_file, line))
  {
    std::smatch match;
    if(std::regex_match(line, match, archive_regex))
    {
      if(!found_archive)
        archive_line = i;
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

  if(archive_line == -1)
  {
    for(const auto& [plugin, enabled] : plugins_)
    {
      if(enabled)
        out_file << "content=" + plugin + "\n";
    }
  }
  for(const auto& [i, line] : str::enumerate_view(lines))
  {
    if(i == archive_line)
    {
      for(const auto& [plugin, enabled] : plugins_)
      {
        if(enabled)
          out_file << "fallback-archive=" + plugin + "\n";
      }
    }
    out_file << line << "\n";
  }
}

void OpenMwArchiveDeployer::updatePluginTags() {}

bool OpenMwArchiveDeployer::initPluginFile()
{
  const sfs::path plugin_file_path = dest_path_ / plugin_file_name_;
  if(sfs::exists(plugin_file_path))
    return false;

  const sfs::path config_file_path = dest_path_ / OPEN_MW_CONFIG_FILE_NAME;
  std::ifstream in_file(config_file_path);
  if(!in_file.is_open())
    throw std::runtime_error(std::format("Error: Could not open '{}'.", config_file_path.string()));

  std::string line;
  std::regex archive_regex(R"(^fallback-archive=(.*?\.[bB][sS][aA]))");
  while(getline(in_file, line))
  {
    std::smatch match;
    if(std::regex_match(line, match, archive_regex))
      plugins_.emplace_back(match[1], true);
  }

  PluginDeployer::writePlugins();
  return true;
}
