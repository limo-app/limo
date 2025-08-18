#include "plugindeployer.h"
#include "pathutils.h"
#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <numeric>
#include <ranges>
#include <regex>

namespace sfs = std::filesystem;
namespace str = std::ranges;
namespace pu = path_utils;


PluginDeployer::PluginDeployer(const sfs::path& source_path,
                               const sfs::path& dest_path,
                               const std::string& name) : Deployer(source_path, dest_path, name)
{
  // make sure no hard link related checks are performed
  deploy_mode_ = copy;
  type_ = "Plugin Deployer";
  is_autonomous_ = true;
}

std::map<int, unsigned long> PluginDeployer::deploy(std::optional<ProgressNode*> progress_node)
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating plugins...", name_));
  restoreUndeployBackupIfExists();
  updatePlugins();
  updatePluginTags();
  updateSourceMods();
  return {};
}

std::map<int, unsigned long> PluginDeployer::deploy(const std::vector<int>& loadorder,
                                                    std::optional<ProgressNode*> progress_node)
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating plugins...", name_));
  restoreUndeployBackupIfExists();
  updatePlugins();
  updatePluginTags();
  updateSourceMods();
  return {};
}

void PluginDeployer::swapChild(int from_index, int to_index)
{
  if(to_index == from_index || to_index < 0 || to_index >= plugins_.size())
    return;
  iter_swap(plugins_.begin() + to_index, plugins_.begin() + from_index);
  if(tags_.size() == plugins_.size())
  {
    iter_swap(tags_.begin() + to_index, tags_.begin() + from_index);
  }
  writePluginTags();
  writePlugins();
}

void PluginDeployer::setModStatus(int mod_id, bool status)
{
  if(mod_id >= plugins_.size() || mod_id < 0)
    return;
  plugins_[mod_id].second = status;
  writePlugins();
}

std::vector<std::vector<int>> PluginDeployer::getConflictGroups() const
{
  std::vector<int> group(plugins_.size());
  std::iota(group.begin(), group.end(), 0);
  return { group };
}

std::vector<std::string> PluginDeployer::getModNames() const
{
  std::vector<std::string> names{};
  names.reserve(plugins_.size());
  for(int i = 0; i < plugins_.size(); i++)
    names.push_back(plugins_[i].first);
  return names;
}

void PluginDeployer::addProfile(int source)
{
  if(num_profiles_ == 0)
  {
    num_profiles_++;
    saveSettings();
    return;
  }
  if(source >= 0 && source <= num_profiles_ && num_profiles_ > 1 && source != current_profile_)
  {
    sfs::copy(dest_path_ / (hideFile(plugin_file_name_) + EXTENSION + std::to_string(source)),
              dest_path_ /
                (hideFile(plugin_file_name_) + EXTENSION + std::to_string(num_profiles_)));
  }
  else
  {
    sfs::copy(dest_path_ / plugin_file_name_,
              dest_path_ /
                (hideFile(plugin_file_name_) + EXTENSION + std::to_string(num_profiles_)));
  }
  num_profiles_++;
  saveSettings();
}

void PluginDeployer::removeProfile(int profile)
{
  if(profile >= num_profiles_ || profile < 0)
    return;
  std::string plugin_file = hideFile(plugin_file_name_) + EXTENSION + std::to_string(profile);
  if(profile == current_profile_)
    setProfile(0);
  else if(profile < current_profile_)
    setProfile(current_profile_ - 1);
  sfs::remove(dest_path_ / plugin_file);
  num_profiles_--;
  saveSettings();
}

void PluginDeployer::setProfile(int profile)
{
  if(profile >= num_profiles_ || profile < 0 || profile == current_profile_)
    return;
  if(!sfs::exists(dest_path_ / plugin_file_name_) ||
     !sfs::exists(dest_path_ / (hideFile(plugin_file_name_) + EXTENSION + std::to_string(profile))))
  {
    resetSettings();
    return;
  }
  sfs::rename(dest_path_ / plugin_file_name_,
              dest_path_ /
                (hideFile(plugin_file_name_) + EXTENSION + std::to_string(current_profile_)));
  sfs::rename(dest_path_ / (hideFile(plugin_file_name_) + EXTENSION + std::to_string(profile)),
              dest_path_ / plugin_file_name_);
  current_profile_ = profile;
  saveSettings();
  loadPlugins();
  updatePlugins();
}

void PluginDeployer::setConflictGroups(const std::vector<std::vector<int>>& newConflict_groups)
{
  log_(Log::LOG_DEBUG,
       std::string("WARNING: You are trying to set a load order for an autonomous deployer. ") +
         "This will have no effect.");
}

int PluginDeployer::getNumMods()
{
  return plugins_.size();
}

std::shared_ptr<TreeItem<DeployerEntry>> PluginDeployer::getLoadorder()
{
  auto loadorder = std::make_shared<TreeItem<DeployerEntry>>(std::make_shared<DeployerEntry>(true, "Root"), nullptr);
  for(const auto& [plugin, enabled] : plugins_)
  {
    auto iter = source_mods_.find(plugin);
    int id = -1;
    if(iter != source_mods_.end())
      id = iter->second;
    loadorder->emplace_back(make_shared<DeployerModInfo>(false, plugin, "", id, enabled));
  }
  return loadorder;
}

bool PluginDeployer::addMod(int mod_id, bool enabled, bool update_conflicts)
{
  log_(Log::LOG_DEBUG,
       std::string("WARNING: You are trying to add a mod to an autonomous deployer. ") +
         "This will have no effect.");
  return false;
}

bool PluginDeployer::removeMod(int mod_id)
{
  log_(Log::LOG_DEBUG,
       std::string("WARNING: You are trying to remove a mod from an autonomous deployer. ") +
         "This will have no effect.");
  return false;
}

bool PluginDeployer::hasMod(int mod_id)
{
  return false;
}

bool PluginDeployer::swapMod(int old_id, int new_id)
{
  log_(Log::LOG_DEBUG,
       std::string("WARNING: You are trying to swap a mod in an autonomous deployer. ") +
         "This will have no effect");
  return false;
}

std::vector<ConflictInfo> PluginDeployer::getFileConflicts(
  int mod_id,
  bool show_disabled,
  std::optional<ProgressNode*> progress_node)
{
  if(progress_node)
  {
    (*progress_node)->setTotalSteps(1);
    (*progress_node)->advance();
  }
  return {};
}

std::unordered_set<int> PluginDeployer::getModConflicts(int mod_id,
                                                        std::optional<ProgressNode*> progress_node)
{
  if(progress_node)
  {
    (*progress_node)->setTotalSteps(1);
    (*progress_node)->advance();
  }
  return {};
}

void PluginDeployer::sortModsByConflicts(std::optional<ProgressNode*> progress_node)
{
  if(progress_node)
  {
    (*progress_node)->setTotalSteps(1);
    (*progress_node)->advance();
  }
}

void PluginDeployer::cleanup()
{
  for(int i = 0; i < num_profiles_; i++)
  {
    sfs::path plugin_path =
      dest_path_ / (hideFile(plugin_file_name_) + EXTENSION + std::to_string(i));
    sfs::remove(plugin_path);
  }
  current_profile_ = 0;
  num_profiles_ = 1;
  sfs::remove(dest_path_ / config_file_name_);
}

std::vector<std::vector<std::string>> PluginDeployer::getAutoTags()
{
  return tags_;
}

std::map<std::string, int> PluginDeployer::getAutoTagMap()
{
  return {};
}

std::vector<std::pair<std::filesystem::path, int>> PluginDeployer::getExternallyModifiedFiles(
  std::optional<ProgressNode*> progress_node) const
{
  if(progress_node)
  {
    (*progress_node)->setTotalSteps(1);
    (*progress_node)->advance();
  }
  return {};
}

void PluginDeployer::keepOrRevertFileModifications(const FileChangeChoices& changes_to_keep) {}

void PluginDeployer::updateDeployedFilesForMod(int mod_id,
                                               std::optional<ProgressNode*> progress_node) const
{
  if(progress_node)
  {
    (*progress_node)->setTotalSteps(1);
    (*progress_node)->advance();
  }
}

void PluginDeployer::fixInvalidLinkDeployMode() {}

void PluginDeployer::setDeployMode(DeployMode deploy_mode)
{
  deploy_mode_ = copy;
}

int PluginDeployer::getDeployPriority() const
{
  return 1;
}

bool PluginDeployer::supportsFileConflicts() const
{
  return false;
}

bool PluginDeployer::supportsFileBrowsing() const
{
  return false;
}

bool PluginDeployer::idsAreSourceReferences() const
{
  return true;
}

std::vector<std::vector<int>> PluginDeployer::getValidModActions() const
{
  std::vector<std::vector<int>> valid_actions;
  for(int _ = 0; _ < plugins_.size(); _++)
    valid_actions.push_back({});
  return valid_actions;
}

void PluginDeployer::updatePlugins()
{
  std::vector<std::string> plugin_files;
  std::vector<std::pair<std::string, bool>> new_plugins;
  for(const auto& dir_entry : sfs::directory_iterator(source_path_))
  {
    if(dir_entry.is_directory())
      continue;
    const std::string file_name = dir_entry.path().filename().string();
    if(std::regex_match(file_name, plugin_regex_))
      plugin_files.push_back(file_name);
  }
  for(auto it = plugins_.begin(); it != plugins_.end(); it++)
  {
    if(str::find_if(plugin_files, [&it](const auto& s) { return it->first == s; }) !=
       plugin_files.end())
      new_plugins.emplace_back(*it);
  }
  for(auto it = plugin_files.begin(); it != plugin_files.end(); it++)
  {
    if(str::find_if(new_plugins, [&it](auto& p) { return p.first == *it; }) == new_plugins.end())
      new_plugins.emplace_back(*it, true);
  }
  plugins_ = new_plugins;
  writePlugins();
}

void PluginDeployer::loadPlugins()
{
  plugins_.clear();
  std::string line;
  std::ifstream plugin_file;
  plugin_file.open(dest_path_ / plugin_file_name_);
  if(!plugin_file.is_open())
    throw std::runtime_error("Could not open " + plugin_file_name_ +
                             "!\nMake sure you have launched the game at least once.");

  while(getline(plugin_file, line))
  {
    std::smatch match;
    if(std::regex_match(line, match, plugin_file_line_regex_))
      plugins_.emplace_back(match[2], match[1] == "*");
  }
  plugin_file.close();
}

void PluginDeployer::writePlugins() const
{
  std::ofstream plugin_file(dest_path_ / plugin_file_name_);
  if(!plugin_file.is_open())
    throw std::runtime_error("Could not open " + plugin_file_name_ + "!");
  for(const auto& [name, status] : plugins_)
    plugin_file << (status ? "*" : "") << name << "\n";
  plugin_file.close();
}

void PluginDeployer::saveSettings() const
{
  Json::Value settings;
  settings["num_profiles"] = num_profiles_;
  settings["current_profile"] = current_profile_;
  sfs::path settings_file_path = dest_path_ / config_file_name_;
  std::ofstream file(settings_file_path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not write to \"" + settings_file_path.string() + "\".");
  file << settings;
  file.close();
}

void PluginDeployer::loadSettings()
{
  Json::Value settings;
  sfs::path settings_file_path = dest_path_ / config_file_name_;
  if(!sfs::exists(settings_file_path))
  {
    resetSettings();
    return;
  }
  std::ifstream file(settings_file_path, std::fstream::binary);
  if(!file.is_open())
  {
    resetSettings();
    return;
  }
  file >> settings;
  file.close();
  if(!settings.isMember("num_profiles") || !settings.isMember("current_profile"))
  {
    resetSettings();
    return;
  }
  num_profiles_ = settings["num_profiles"].asInt();
  current_profile_ = settings["current_profile"].asInt();
}

void PluginDeployer::resetSettings()
{
  num_profiles_ = 1;
  current_profile_ = 0;
}

void PluginDeployer::writePluginTags() const
{
  Json::Value json;
  for(int i = 0; i < tags_.size(); i++)
  {
    for(int j = 0; j < tags_[i].size(); j++)
      json[i][j] = tags_.at(i).at(j);
  }

  const sfs::path tag_file_path = dest_path_ / tags_file_name_;
  std::ofstream file(tag_file_path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not write to \"" + tag_file_path.string() + "\".");
  file << json;
  file.close();
}

void PluginDeployer::restoreUndeployBackupIfExists()
{
  const std::string plugin_backup_path =
    dest_path_ / (hideFile(plugin_file_name_) + UNDEPLOY_BACKUP_EXTENSION);
  if(sfs::exists(plugin_backup_path))
  {
    log_(Log::LOG_DEBUG, std::format("Deployer '{}': Restoring undeploy backup.", name_));
    sfs::remove(dest_path_ / plugin_file_name_);
    sfs::rename(plugin_backup_path, dest_path_ / plugin_file_name_);
    loadPlugins();
  }
}

void PluginDeployer::updateSourceMods()
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Finding source mods...", name_));
  source_mods_.clear();

  auto deployed_source_path = getRootOfTargetDirectory(source_path_);
  if(deployed_source_path)
    log_(Log::LOG_DEBUG, std::format("Source path: '{}'", deployed_source_path->string()));
  else
  {
    log_(Log::LOG_ERROR,
         std::format("Deployer '{}': Could not find deployed files at '{}'",
                     name_,
                     deployed_source_path->string()));
    return;
  }
  auto deployed_files = loadDeployedFiles({}, *deployed_source_path);
  const sfs::path relative_path(pu::getRelativePath(source_path_, *deployed_source_path));
  for(const auto& [name, _] : plugins_)
  {
    auto iter = deployed_files.find((relative_path / name).string());
    if(iter != deployed_files.end())
      source_mods_[name] = iter->second;
  }
  writeSourceMods();
}

void PluginDeployer::writeSourceMods() const
{
  Json::Value json_object;
  for(const auto& [i, pair] : str::enumerate_view(source_mods_))
  {
    const auto& [plugin, source] = pair;
    json_object["source_mods"][(int)i]["plugin"] = plugin;
    json_object["source_mods"][(int)i]["source"] = source;
  }

  std::ofstream file(dest_path_ / source_mods_file_name_, std::ios::binary);
  if(!file.is_open())
  {
    log_(Log::LOG_ERROR,
         std::format(
           "Deployed '{}': Failed to write mod sources to '{}'", name_, dest_path_.string()));
    return;
  }
  file << json_object;
}

void PluginDeployer::readSourceMods()
{
  const sfs::path dest_path = dest_path_ / source_mods_file_name_;
  if(!sfs::exists(dest_path))
    return;

  std::ifstream file(dest_path, std::ios::binary);
  if(!file.is_open())
  {
    log_(Log::LOG_ERROR,
         std::format(
           "Deployed '{}': Failed to read mod sources from '{}'", name_, dest_path_.string()));
    return;
  }
  Json::Value json_object;
  file >> json_object;
  file.close();
  source_mods_.clear();
  for(int i = 0; i < json_object["source_mods"].size(); i++)
    source_mods_[json_object["source_mods"][i]["plugin"].asString()] =
      json_object["source_mods"][(int)i]["source"].asInt();
}

std::optional<sfs::path> PluginDeployer::getRootOfTargetDirectory(sfs::path target) const
{
  while(target.string() != "/" && !target.empty())
  {
    if(sfs::exists(target / deployed_files_name_))
      return { target };
    sfs::path new_target;
    const int length = pu::getPathLength(target);
    for(const auto& [i, part] : str::enumerate_view(target))
    {
      if(i == length - 1)
        break;
      new_target /= part;
    }
    target = new_target;
  }
  return {};
}

std::string PluginDeployer::hideFile(const std::string& name)
{
  return name.starts_with('.') ? name : "." + name;
}
