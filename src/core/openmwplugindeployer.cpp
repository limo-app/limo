#include "openmwplugindeployer.h"
#include "pathutils.h"
#include <format>
#include <fstream>
#include <json/json.h>
#include <ranges>

namespace sfs = std::filesystem;
namespace str = std::ranges;
namespace pu = path_utils;


OpenMwPluginDeployer::OpenMwPluginDeployer(const sfs::path& source_path,
                                           const sfs::path& dest_path,
                                           const std::string& name) :
  LootDeployer(source_path, dest_path, name, false, false)
{
  // make sure no hard link related checks are performed
  deploy_mode_ = copy;
  type_ = "OpenMW Plugin Deployer";
  is_autonomous_ = true;
  app_type_ = loot::GameType::openmw;
  plugin_regex_ =
    std::regex(R"(.*\.(?:es[pm]|omwscripts|omwaddon|omwgame)$)", std::regex_constants::icase);
  plugin_file_line_regex_ = std::regex(
    R"(^\s*(\*?)([^#]*\.(?:es[pm]|omwscripts|omwaddon|omwgame))(\r?))", std::regex_constants::icase);
  config_file_name_ = ".plugin_config";
  source_mods_file_name_ = ".plugin_mod_sources";
  plugin_file_name_ = ".plugins.txt";
  tags_file_name_ = ".omwplugin_tags";
  bool initialized_plugins = initPluginFile();
  if(!initialized_plugins)    loadPlugins();
  readPluginTags();
  updatePlugins();
  if(initialized_plugins)
    updatePluginTagsPrivate();
  if(sfs::exists(dest_path_ / config_file_name_))
    loadSettings();
  readSourceMods();
  writePluginsPrivate();
  writePluginTagsPrivate();
  saveSettings();
}

void OpenMwPluginDeployer::unDeploy(std::optional<ProgressNode*> progress_node)
{
  const std::string plugin_backup_path =
    dest_path_ / ("." + plugin_file_name_ + UNDEPLOY_BACKUP_EXTENSION);
  if(!pu::exists(plugin_backup_path))
    sfs::copy(dest_path_ / plugin_file_name_, plugin_backup_path);

  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating plugins...", name_));
  updatePlugins();
}

std::vector<std::vector<int>> OpenMwPluginDeployer::getConflictGroups() const
{
  std::vector<std::vector<int>> groups;
  for(int i = 0; i < 3; i++)
    groups.push_back({});

  std::regex script_regex(R"(.*\.omwscripts$)", std::regex_constants::icase);
  for(const auto& [i, pair] : str::enumerate_view(plugins_))
  {
    const auto& [plugin, enabled] = pair;
    if(std::regex_match(plugin, script_regex))
      groups[0].push_back(i);
    else if(groundcover_plugins_.contains(plugin))
      groups[1].push_back(i);
    else
      groups[2].push_back(i);
  }

  return groups;
}

std::map<std::string, int> OpenMwPluginDeployer::getAutoTagMap()
{
  return { { GROUNDCOVER_TAG, num_groundcover_plugins_ },
           { OPENMW_TAG, num_openmw_plugins_ },
           { ES_PLUGIN_TAG, num_es_plugins_ } };
}

void OpenMwPluginDeployer::sortModsByConflicts(std::optional<ProgressNode*> progress_node)
{
  LootDeployer::sortModsByConflicts(progress_node);

  auto groups = getConflictGroups();
  std::vector<std::pair<std::string, bool>> new_plugins;
  new_plugins.reserve(plugins_.size());
  for(const auto& group : groups)
  {
    for(int mod_id : group)
      new_plugins.push_back(plugins_[mod_id]);
  }
  plugins_ = new_plugins;
  updatePluginTags();
  writePlugins();
}

std::vector<std::pair<std::string, std::string>> OpenMwPluginDeployer::getModActions() const
{
  return { { "Add Groundcover Tag", "tag-new" }, { "Remove Groundcover Tag", "tag-delete" } };
}

std::vector<std::vector<int>> OpenMwPluginDeployer::getValidModActions() const
{
  std::vector<std::vector<int>> valid_actions;
  for(const auto& [plugin, enabled] : plugins_)
  {
    auto iter = tag_map_.find(plugin);
    if(iter != tag_map_.end() && iter->second.contains(SCRIPTS_PLUGIN_TAG))
      valid_actions.push_back({});
    else if(!groundcover_plugins_.contains(plugin))
      valid_actions.push_back({ 0 });
    else
      valid_actions.push_back({ 1 });
  }
  return valid_actions;
}

void OpenMwPluginDeployer::applyModAction(int action, int mod_id)
{
  if(action == ACTION_ADD_GROUNDCOVER_TAG)
  {
    groundcover_plugins_.insert(plugins_[mod_id].first);
    num_groundcover_plugins_++;
  }
  else if(action == ACTION_REMOVE_GROUNDCOVER_TAG)
  {
    groundcover_plugins_.erase(plugins_[mod_id].first);
    num_groundcover_plugins_--;
  }
  else
    log_(Log::LOG_DEBUG, std::format("Invalid mod action: {}", action));

  updateTagVector();
  writePluginTags();
  writePlugins();
}

void OpenMwPluginDeployer::addProfile(int source)
{
  // we don't want the changes made to profiles by LootDeployer
  PluginDeployer::addProfile(source);
}

void OpenMwPluginDeployer::removeProfile(int profile)
{
  // we don't want the changes made to profiles by LootDeployer
  PluginDeployer::removeProfile(profile);
}

void OpenMwPluginDeployer::setProfile(int profile)
{
  // we don't want the changes made to profiles by LootDeployer
  PluginDeployer::setProfile(profile);
}

void OpenMwPluginDeployer::writePlugins() const
{
  writePluginsPrivate();
}

bool OpenMwPluginDeployer::initPluginFile()
{
  const sfs::path plugin_file_path = dest_path_ / plugin_file_name_;
  if(sfs::exists(plugin_file_path))
    return false;

  const sfs::path config_file_path = dest_path_ / OPEN_MW_CONFIG_FILE_NAME;
  std::ifstream in_file(config_file_path);
  if(!in_file.is_open())
    throw std::runtime_error(std::format("Error: Could not open '{}'.", config_file_path.string()));

  std::string line;
  std::regex plugin_regex(R"(^content=(.*\.(?:es[pm]|omwscripts|omwaddon|omwgame)))",
                          std::regex_constants::icase);
  std::regex groundcover_regex(R"(^groundcover=(.*\.(?:es[pm]|omwscripts|omwaddon|omwgame)))",
                               std::regex_constants::icase);
  num_groundcover_plugins_ = 0;
  while(getline(in_file, line))
  {
    std::smatch match;
    if(std::regex_match(line, match, plugin_regex))
      plugins_.emplace_back(match[1], true);
    else if(std::regex_match(line, match, groundcover_regex))
    {
      plugins_.emplace_back(match[1], true);
      groundcover_plugins_.insert(match[1]);
      num_groundcover_plugins_++;
    }
  }

  updateTagVector();
  PluginDeployer::writePlugins();
  return true;
}

void OpenMwPluginDeployer::readPluginTags()
{
  const sfs::path tag_file_path = dest_path_ / tags_file_name_;
  if(!sfs::exists(tag_file_path))
  {
    updatePluginTagsPrivate();
    return;
  }
  tag_map_.clear();
  groundcover_plugins_.clear();
  num_groundcover_plugins_ = 0;
  num_openmw_plugins_ = 0;
  num_es_plugins_ = 0;
  num_scripts_plugins_ = 0;
  std::ifstream file(tag_file_path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not read from \"" + tag_file_path.string() + "\".");
  Json::Value json;
  file >> json;
  file.close();
  for(int i = 0; i < json.size(); i++)
  {
    const std::string plugin = json[i]["plugin"].asString();
    tag_map_[plugin] = {};
    for(int j = 0; j < json[i]["tags"].size(); j++)
    {
      const std::string tag = json[i]["tags"][j].asString();
      if(tag == GROUNDCOVER_TAG)
      {
        groundcover_plugins_.insert(plugin);
        num_groundcover_plugins_++;
      }
      else if(tag == OPENMW_TAG)
      {
        tag_map_[plugin].insert(tag);
        num_openmw_plugins_++;
      }
      else if(tag == ES_PLUGIN_TAG)
      {
        tag_map_[plugin].insert(tag);
        num_es_plugins_++;
      }
      else if(tag == SCRIPTS_PLUGIN_TAG)
      {
        tag_map_[plugin].insert(tag);
        num_scripts_plugins_++;
      }
    }
  }
  updateTagVector();
}

void OpenMwPluginDeployer::writePluginTags() const
{
  writePluginTagsPrivate();
}

void OpenMwPluginDeployer::updatePluginTags()
{
  updatePluginTagsPrivate();
}

void OpenMwPluginDeployer::updateTagVector()
{
  tags_.clear();
  for(const auto& [plugin, _] : plugins_)
  {
    tags_.push_back({});
    auto iter = tag_map_.find(plugin);
    if(iter != tag_map_.end())
    {
      for(const auto& tag : iter->second)
        tags_.back().push_back(tag);
    }
    if(groundcover_plugins_.contains(plugin))
      tags_.back().push_back(GROUNDCOVER_TAG);
  }
}

void OpenMwPluginDeployer::updatePluginTagsPrivate()
{
  std::regex omw_regex(R"(.*\.(?:omwscripts|omwaddon|omwgame))", std::regex_constants::icase);
  std::regex script_regex(R"(.*\.omwscripts)", std::regex_constants::icase);
  std::regex es_regex(R"(.*\.es[pm])", std::regex_constants::icase);
  num_openmw_plugins_ = 0;
  num_es_plugins_ = 0;
  num_scripts_plugins_ = 0;
  for(const auto& [i, pair] : str::enumerate_view(plugins_))
  {
    const auto& [plugin, _] = pair;
    if(std::regex_match(plugin, omw_regex))
    {
      tag_map_[plugin].insert(OPENMW_TAG);
      num_openmw_plugins_++;
    }
    if(std::regex_match(plugin, es_regex))
    {
      tag_map_[plugin].insert(ES_PLUGIN_TAG);
      num_es_plugins_++;
    }
    if(std::regex_match(plugin, script_regex))
    {
      tag_map_[plugin].insert(SCRIPTS_PLUGIN_TAG);
      num_scripts_plugins_++;
    }
  }
  updateTagVector();
  writePluginTagsPrivate();
}

void OpenMwPluginDeployer::writePluginTagsPrivate() const
{
  Json::Value json;
  for(const auto& [i, pair] : str::enumerate_view(tag_map_))
  {
    const auto& [plugin, tags] = pair;
    json[(int)i]["plugin"] = plugin;
    for(const auto& [j, tag] : str::enumerate_view(tags))
      json[(int)i]["tags"][(int)j] = tag;
    if(groundcover_plugins_.contains(plugin))
      json[(int)i]["tags"][json[(int)i]["tags"].size()] = GROUNDCOVER_TAG;
  }

  const sfs::path tag_file_path = dest_path_ / tags_file_name_;
  std::ofstream file(tag_file_path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not write to \"" + tag_file_path.string() + "\".");
  file << json;
  file.close();
}

void OpenMwPluginDeployer::writePluginsToOpenMwConfig(const std::string& line_prefix,
                                                      const std::regex& line_regex,
                                                      std::function<bool(int)> plugin_filter) const
{
  const sfs::path plugin_file_path = dest_path_ / OPEN_MW_CONFIG_FILE_NAME;
  std::ifstream in_file(plugin_file_path);
  if(!in_file.is_open())
    throw std::runtime_error(std::format("Error: Could not open '{}'.", plugin_file_path.string()));

  std::vector<std::string> lines;
  int target_line = -1;
  bool found_target = false;
  std::string line;
  int i = 0;
  while(getline(in_file, line))
  {
    std::smatch match;
    if(std::regex_match(line, match, line_regex))
    {
      if(!found_target)
        target_line = i;
      found_target = true;
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
    if(i == target_line)
    {
      for(const auto& [i, pair] : str::enumerate_view(plugins_))
      {
        if(plugin_filter(i))
          out_file << line_prefix + pair.first + "\n";
      }
    }
    out_file << line << "\n";
  }
  if(target_line == -1 || target_line >= lines.size())
  {
    for(const auto& [i, pair] : str::enumerate_view(plugins_))
    {
      if(plugin_filter(i))
        out_file << line_prefix + pair.first + "\n";
    }
  }
}

void OpenMwPluginDeployer::writePluginsPrivate() const
{
  PluginDeployer::writePlugins();

  writePluginsToOpenMwConfig("content=",
                             std::regex(R"(^content=.*)"),
                             [this](int i) {
                               return plugins_[i].second &&
                                      !groundcover_plugins_.contains(plugins_[i].first);
                             });
  writePluginsToOpenMwConfig("groundcover=",
                             std::regex(R"(^groundcover=.*)"),
                             [this](int i) {
                               return plugins_[i].second &&
                                      groundcover_plugins_.contains(plugins_[i].first);
                             });
}
