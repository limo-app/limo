#include "lootdeployer.h"
#include "pathutils.h"
#include <chrono>
#include <cpr/cpr.h>
#include <fstream>
#include <iostream>
#include <numeric>
#include <ranges>
#include <regex>
#include <set>

namespace sfs = std::filesystem;
namespace str = std::ranges;
namespace pu = path_utils;


LootDeployer::LootDeployer(const sfs::path& source_path,
                           const sfs::path& dest_path,
                           const std::string& name,
                           bool init_tags) : Deployer(source_path, dest_path, name)
{
  LIST_URLS = DEFAULT_LIST_URLS;
  type_ = "Loot Deployer";
  is_autonomous_ = true;
  updateAppType();
  setupPluginFiles();
  loadPlugins();
  updatePlugins();
  if(sfs::exists(dest_path_ / CONFIG_FILE_NAME))
    loadSettings();
  if(init_tags)
    readPluginTags();
  // make sure no hard link related checks are performed
  deploy_mode_ = copy;
}

std::map<int, unsigned long> LootDeployer::deploy(std::optional<ProgressNode*> progress_node)
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating plugins...", name_));
  updatePlugins();
  updatePluginTags();
  return {};
}

std::map<int, unsigned long> LootDeployer::deploy(const std::vector<int>& loadorder,
                                                  std::optional<ProgressNode*> progress_node)
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating plugins...", name_));
  updatePlugins();
  updatePluginTags();
  return {};
}

void LootDeployer::changeLoadorder(int from_index, int to_index)
{
  if(to_index == from_index)
    return;
  if(to_index < 0 || to_index >= plugins_.size())
    return;
  if(to_index < from_index)
    std::rotate(plugins_.begin() + to_index,
                plugins_.begin() + from_index,
                plugins_.begin() + from_index + 1);
  else
    std::rotate(plugins_.begin() + from_index,
                plugins_.begin() + from_index + 1,
                plugins_.begin() + to_index + 1);
  writePlugins();
}

void LootDeployer::setModStatus(int mod_id, bool status)
{
  if(mod_id >= plugins_.size() || mod_id < 0)
    return;
  plugins_[mod_id].second = status;
  writePlugins();
}

std::vector<std::vector<int>> LootDeployer::getConflictGroups() const
{
  std::vector<int> group(plugins_.size());
  std::iota(group.begin(), group.end(), 0);
  return { group };
}

std::vector<std::string> LootDeployer::getModNames() const
{
  std::vector<std::string> names{};
  names.reserve(plugins_.size());
  for(int i = 0; i < plugins_.size(); i++)
    names.push_back(plugins_[i].first);
  return names;
}

void LootDeployer::addProfile(int source)
{
  if(num_profiles_ == 0)
  {
    num_profiles_++;
    saveSettings();
    return;
  }
  if(source >= 0 && source <= num_profiles_ && num_profiles_ > 1)
  {
    sfs::copy(dest_path_ / ("." + plugin_file_name_ + EXTENSION + std::to_string(source)),
              dest_path_ / ("." + plugin_file_name_ + EXTENSION + std::to_string(num_profiles_)));
    sfs::copy(dest_path_ / ("." + LOADORDER_FILE_NAME + EXTENSION + std::to_string(source)),
              dest_path_ / ("." + LOADORDER_FILE_NAME + EXTENSION + std::to_string(num_profiles_)));
  }
  else
  {
    sfs::copy(dest_path_ / plugin_file_name_,
              dest_path_ / ("." + plugin_file_name_ + EXTENSION + std::to_string(num_profiles_)));
    sfs::copy(dest_path_ / LOADORDER_FILE_NAME,
              dest_path_ / ("." + LOADORDER_FILE_NAME + EXTENSION + std::to_string(num_profiles_)));
  }
  num_profiles_++;
  saveSettings();
}

void LootDeployer::removeProfile(int profile)
{
  if(profile >= num_profiles_ || profile < 0)
    return;
  std::string plugin_file = "." + plugin_file_name_ + EXTENSION + std::to_string(profile);
  std::string loadorder_file = "." + LOADORDER_FILE_NAME + EXTENSION + std::to_string(profile);
  if(profile == current_profile_)
    setProfile(profile == 0 ? num_profiles_ - 2 : 0);
  if(sfs::exists(dest_path_ / plugin_file))
    sfs::remove(dest_path_ / plugin_file);
  if(sfs::exists(dest_path_ / loadorder_file))
    sfs::remove(dest_path_ / loadorder_file);
  num_profiles_--;
  saveSettings();
}

void LootDeployer::setProfile(int profile)
{
  if(profile >= num_profiles_ || profile < 0 || profile == current_profile_)
    return;
  if(!sfs::exists(dest_path_ / plugin_file_name_) ||
     !sfs::exists(dest_path_ / LOADORDER_FILE_NAME) ||
     !sfs::exists(dest_path_ / ("." + plugin_file_name_ + EXTENSION + std::to_string(profile))) ||
     !sfs::exists(dest_path_ / ("." + LOADORDER_FILE_NAME + EXTENSION + std::to_string(profile))))
  {
    resetSettings();
    return;
  }
  sfs::rename(dest_path_ / plugin_file_name_,
              dest_path_ /
                ("." + plugin_file_name_ + EXTENSION + std::to_string(current_profile_)));
  sfs::rename(dest_path_ / LOADORDER_FILE_NAME,
              dest_path_ /
                ("." + LOADORDER_FILE_NAME + EXTENSION + std::to_string(current_profile_)));
  sfs::rename(dest_path_ / ("." + plugin_file_name_ + EXTENSION + std::to_string(profile)),
              dest_path_ / plugin_file_name_);
  sfs::rename(dest_path_ / ("." + LOADORDER_FILE_NAME + EXTENSION + std::to_string(profile)),
              dest_path_ / LOADORDER_FILE_NAME);
  current_profile_ = profile;
  saveSettings();
  loadPlugins();
  updatePlugins();
}

void LootDeployer::setConflictGroups(const std::vector<std::vector<int>>& newConflict_groups)
{
  log_(Log::LOG_DEBUG,
       "WARNING: You are trying to set a load order for an autonomous"
       " deployer. This will have no effect");
}

int LootDeployer::getNumMods() const
{
  return plugins_.size();
}

std::vector<std::tuple<int, bool>> LootDeployer::getLoadorder() const
{
  std::vector<std::tuple<int, bool>> loadorder;
  loadorder.reserve(plugins_.size());
  for(int i = 0; i < plugins_.size(); i++)
    loadorder.emplace_back(i, plugins_[i].second);
  return loadorder;
}

bool LootDeployer::addMod(int mod_id, bool enabled, bool update_conflicts)
{
  log_(Log::LOG_DEBUG,
       "WARNING: You are trying to add a mod to an autonomous"
       " deployer. This will have no effect");
  return false;
}

bool LootDeployer::removeMod(int mod_id)
{
  log_(Log::LOG_DEBUG,
       "WARNING: You are trying to remove a mod from an autonomous"
       " deployer. This will have no effect");
  return false;
}

bool LootDeployer::hasMod(int mod_id) const
{
  return false;
}

bool LootDeployer::swapMod(int old_id, int new_id)
{
  log_(Log::LOG_DEBUG,
       "WARNING: You are trying to swap a mod in an autonomous"
       " deployer. This will have no effect");
  return false;
}

std::vector<ConflictInfo> LootDeployer::getFileConflicts(
  int mod_id,
  bool show_disabled,
  std::optional<ProgressNode*> progress_node) const
{
  if(progress_node)
  {
    (*progress_node)->setTotalSteps(1);
    (*progress_node)->advance();
  }
  return {};
}

std::unordered_set<int> LootDeployer::getModConflicts(int mod_id,
                                                      std::optional<ProgressNode*> progress_node)
{
  std::unordered_set<int> conflicts{ mod_id };
  auto loot_handle = loot::CreateGameHandle(app_type_, source_path_, dest_path_);
  std::vector<sfs::path> plugin_paths;
  plugin_paths.reserve(plugins_.size());
  for(const auto& [path, s] : plugins_)
    plugin_paths.emplace_back(source_path_ / path);
  loot_handle->LoadPlugins(plugin_paths, false);
  auto plugin = loot_handle->GetPlugin(plugins_[mod_id].first);
  for(int i = 0; i < plugins_.size(); i++)
  {
    if(i == mod_id)
      continue;
    if(loot_handle->GetPlugin(plugins_[i].first)->DoRecordsOverlap(*plugin))
      conflicts.insert(i);
  }
  return conflicts;
}

void LootDeployer::sortModsByConflicts(std::optional<ProgressNode*> progress_node)
{
  if(progress_node)
  {
    (*progress_node)->addChildren({ 1, 2, 5, 0.2f });
    (*progress_node)->child(0).setTotalSteps(1);
    (*progress_node)->child(1).setTotalSteps(1);
    (*progress_node)->child(2).setTotalSteps(1);
    (*progress_node)->child(3).setTotalSteps(1);
  }
  updateMasterList();
  if(progress_node)
    (*progress_node)->child(0).advance();
  sfs::path master_list_path = dest_path_ / "masterlist.yaml";
  if(!sfs::exists(master_list_path))
    throw std::runtime_error("Could not find masterlist.yaml at '" + master_list_path.string() +
                             "'\n.Try to update the URL in the " +
                             "settings. Alternatively, you can manually download the " +
                             "file and place it in '" + dest_path_.string() + "'.\nYou can " +
                             "disable auto updates in '" +
                             (dest_path_ / CONFIG_FILE_NAME).string() + "'.");
  auto loot_handle = loot::CreateGameHandle(app_type_, source_path_, dest_path_);
  sfs::path user_list_path(dest_path_ / "userlist.yaml");
  if(!sfs::exists(user_list_path))
    user_list_path = "";
  sfs::path prelude_path(dest_path_ / "prelude.yaml");
  if(!sfs::exists(prelude_path))
    prelude_path = "";
  loot_handle->GetDatabase().LoadLists(master_list_path, user_list_path, prelude_path);
  if(progress_node)
    (*progress_node)->child(1).advance();
  std::vector<sfs::path> plugin_paths;
  plugin_paths.reserve(plugins_.size());
  for(const auto& [path, s] : plugins_)
    plugin_paths.emplace_back(source_path_ / path);
  auto sorted_plugins = loot_handle->SortPlugins(plugin_paths);
  if(progress_node)
    (*progress_node)->child(2).advance();
  std::vector<std::pair<std::string, bool>> new_plugins;
  std::set<std::string> conflicting;
  int num_light_plugins = 0;
  int num_master_plugins = 0;
  int num_standard_plugins = 0;
  for(const auto& plugin : sorted_plugins)
  {
    auto iter = str::find_if(plugins_, [plugin](const auto& p) { return p.first == plugin; });
    bool enabled = true;
    if(iter != plugins_.end())
      enabled = iter->second;
    const auto cur_plugin = loot_handle->GetPlugin(plugin);
    if(cur_plugin->IsLightPlugin())
      num_light_plugins++;
    else if(cur_plugin->IsMaster())
      num_master_plugins++;
    else
      num_standard_plugins++;
    new_plugins.emplace_back(plugin, enabled);
    auto masters = cur_plugin->GetMasters();
    for(const auto& master : masters)
    {
      if(!pu::pathExists(master, source_path_) && enabled)
        log_(Log::LOG_WARNING,
             "LOOT: Plugin '" + master + "' is missing but required" + " for '" + plugin + "'");
    }
    auto meta_data = loot_handle->GetDatabase().GetPluginMetadata(plugin);
    if(!meta_data)
      continue;
    auto requirements = meta_data->GetRequirements();
    for(const auto& req : requirements)
    {
      std::string file = static_cast<std::string>(req.GetName());
      if(!pu::pathExists(file, source_path_))
        log_(Log::LOG_WARNING, "LOOT: Requirement '" + file + "' not met for '" + plugin + "'");
    }
  }
  log_(Log::LOG_INFO,
       std::format("LOOT: Total Plugins: {}, Master: {}, Standard: {}, Light: {}",
                   new_plugins.size(),
                   num_master_plugins,
                   num_standard_plugins,
                   num_light_plugins));
  plugins_ = new_plugins;
  writePlugins();
  if(progress_node)
    (*progress_node)->child(3).advance();
}

void LootDeployer::cleanup()
{
  for(int i = 0; i < num_profiles_; i++)
  {
    sfs::path plugin_path = dest_path_ / ("." + plugin_file_name_ + EXTENSION + std::to_string(i));
    sfs::path load_order_path =
      dest_path_ / ("." + LOADORDER_FILE_NAME + EXTENSION + std::to_string(i));
    if(sfs::exists(plugin_path))
      sfs::remove(plugin_path);
    if(sfs::exists(load_order_path))
      sfs::remove(load_order_path);
  }
  current_profile_ = 0;
  num_profiles_ = 1;
  if(sfs::exists(dest_path_ / CONFIG_FILE_NAME))
    sfs::remove(dest_path_ / CONFIG_FILE_NAME);
}

std::vector<std::vector<std::string>> LootDeployer::getAutoTags()
{
  return tags_;
}

std::map<std::string, int> LootDeployer::getAutoTagMap()
{
  return { { LIGHT_PLUGIN, num_light_plugins_ },
           { MASTER_PLUGIN, num_master_plugins_ },
           { STANDARD_PLUGIN, num_standard_plugins_ } };
}

std::vector<std::pair<sfs::path, int>>
LootDeployer::getExternallyModifiedFiles(std::optional<ProgressNode*> progress_node) const
{
  return {};
}

void LootDeployer::keepOrRevertFileModifications(const FileChangeChoices& changes_to_keep) const
{
}

void LootDeployer::updateDeployedFilesForMod(int mod_id,
                                             std::optional<ProgressNode*> progress_node) const
{}

void LootDeployer::updatePlugins()
{
  std::vector<std::string> plugin_files;
  std::vector<std::pair<std::string, bool>> new_plugins;
  for(const auto& dir_entry : sfs::directory_iterator(source_path_))
  {
    if(dir_entry.is_directory())
      continue;
    const std::string file_name = dir_entry.path().filename().string();
    if(std::regex_match(file_name, std::regex(R"(.*\.[eE][sS][pPlLmM]$)")))
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

void LootDeployer::loadPlugins()
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
    if(std::regex_match(line, match, std::regex(R"(^\s*(\*?)([^#]*\.es[plm])(\r?))")))
      plugins_.emplace_back(match[2], match[1] == "*");
  }
  plugin_file.close();
  std::ifstream loadorder_file;
  loadorder_file.open(dest_path_ / LOADORDER_FILE_NAME);
  if(!loadorder_file.is_open())
    throw std::runtime_error("Could not open " + LOADORDER_FILE_NAME +
                             "!\nMake sure you have launched the game at least once.");
  loadorder_file.close();
}

void LootDeployer::writePlugins() const
{
  std::ofstream plugin_file;
  plugin_file.open(dest_path_ / plugin_file_name_);
  if(!plugin_file.is_open())
    throw std::runtime_error("Could not open " + plugin_file_name_ + "!");
  for(const auto& [name, status] : plugins_)
    plugin_file << (status ? "*" : "") << name << "\n";
  plugin_file.close();
  std::ofstream loadorder_file;
  loadorder_file.open(dest_path_ / LOADORDER_FILE_NAME);
  if(!loadorder_file.is_open())
    throw std::runtime_error("Could not open " + LOADORDER_FILE_NAME + "!");
  for(const auto& [name, status] : plugins_)
    loadorder_file << name << "\n";
  loadorder_file.close();
}

void LootDeployer::saveSettings() const
{
  Json::Value settings;
  settings["num_profiles"] = num_profiles_;
  settings["current_profile"] = current_profile_;
  settings["list_download_time"] = list_download_time_;
  settings["auto_update_master_list"] = auto_update_lists_;
  sfs::path settings_file_path = dest_path_ / CONFIG_FILE_NAME;
  std::ofstream file(settings_file_path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not write to \"" + settings_file_path.string() + "\".");
  file << settings;
  file.close();
}

void LootDeployer::loadSettings()
{
  Json::Value settings;
  sfs::path settings_file_path = dest_path_ / CONFIG_FILE_NAME;
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
  if(!settings.isMember("num_profiles") || !settings.isMember("current_profile") ||
     !settings.isMember("list_download_time") || !settings.isMember("auto_update_master_list"))
  {
    resetSettings();
    return;
  }
  num_profiles_ = settings["num_profiles"].asInt();
  current_profile_ = settings["current_profile"].asInt();
  list_download_time_ = settings["list_download_time"].asInt64();
  auto_update_lists_ = settings["auto_update_master_list"].asBool();
}

void LootDeployer::updateAppType()
{
  for(const auto& [type, file] : TYPE_IDENTIFIERS)
  {
    if(pu::pathExists(file, source_path_))
    {
      app_type_ = type;
      plugin_file_name_ = PLUGIN_FILE_NAMES.at(type);
      auto file_name = pu::pathExists(plugin_file_name_, dest_path_);
      if(file_name)
        plugin_file_name_ = *file_name;
      return;
    }
  }
  throw std::runtime_error("Could not identify game type in '" + source_path_.string() + "'");
}

void LootDeployer::updateMasterList()
{
  if(!auto_update_lists_)
    return;
  const auto cur_time = std::chrono::system_clock::now();
  const std::chrono::time_point<std::chrono::system_clock> update_time(
    (std::chrono::seconds(list_download_time_)));
  const auto one_hour_ago = cur_time - std::chrono::hours(1);
  if(update_time >= one_hour_ago && sfs::exists(dest_path_ / "masterlist.yaml"))
    return;

  downloadList(LIST_URLS.at(app_type_), "masterlist.yaml");
  downloadList(PRELUDE_URL, "prelude.yaml");

  list_download_time_ =
    std::chrono::duration_cast<std::chrono::seconds>(cur_time.time_since_epoch()).count();
  saveSettings();
}

void LootDeployer::resetSettings()
{
  num_profiles_ = 1;
  current_profile_ = 0;
  auto_update_lists_ = true;
  list_download_time_ = 0;
}

void LootDeployer::setupPluginFiles()
{
  if(sfs::exists(dest_path_ / plugin_file_name_) && sfs::exists(dest_path_ / LOADORDER_FILE_NAME))
    return;
  updatePlugins();
}

void LootDeployer::updatePluginTags()
{
  tags_.clear();
  auto loot_handle = loot::CreateGameHandle(app_type_, source_path_, dest_path_);
  std::vector<sfs::path> plugin_paths;
  plugin_paths.reserve(plugins_.size());
  for(const auto& [path, s] : plugins_)
    plugin_paths.emplace_back(source_path_ / path);
  loot_handle->LoadPlugins(plugin_paths, false);
  num_light_plugins_ = 0;
  num_master_plugins_ = 0;
  num_standard_plugins_ = 0;
  for(int i = 0; i < plugins_.size(); i++)
  {
    auto plugin = loot_handle->GetPlugin(plugins_[i].first);
    if(plugin->IsLightPlugin())
    {
      num_light_plugins_++;
      tags_.push_back({ LIGHT_PLUGIN });
    }
    else if(plugin->IsMaster())
    {
      num_master_plugins_++;
      tags_.push_back({ MASTER_PLUGIN });
    }
    else
    {
      num_standard_plugins_++;
      tags_.push_back({ STANDARD_PLUGIN });
    }
  }
  writePluginTags();
}

void LootDeployer::writePluginTags() const
{
  Json::Value json;
  for(int i = 0; i < tags_.size(); i++)
  {
    for(int j = 0; j < tags_[i].size(); j++)
      json[i][j] = tags_.at(i).at(j);
  }

  const sfs::path tag_file_path = dest_path_ / TAGS_FILE_NAME;
  std::ofstream file(tag_file_path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not write to \"" + tag_file_path.string() + "\".");
  file << json;
  file.close();
}

void LootDeployer::readPluginTags()
{
  const sfs::path tag_file_path = dest_path_ / TAGS_FILE_NAME;
  if(!sfs::exists(tag_file_path))
  {
    updatePluginTags();
    return;
  }
  tags_.clear();
  num_light_plugins_ = 0;
  num_master_plugins_ = 0;
  num_standard_plugins_ = 0;
  std::ifstream file(tag_file_path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not read from \"" + tag_file_path.string() + "\".");
  Json::Value json;
  file >> json;
  file.close();
  for(int i = 0; i < json.size(); i++)
  {
    tags_.push_back({});
    for(int j = 0; j < json[i].size(); j++)
    {
      const std::string tag = json[i][j].asString();
      tags_[i].push_back(tag);
      if(tag == LIGHT_PLUGIN)
        num_light_plugins_++;
      else if(tag == MASTER_PLUGIN)
        num_master_plugins_++;
      else if(tag == STANDARD_PLUGIN)
        num_standard_plugins_++;
    }
  }
  if(tags_.size() != plugins_.size())
    updatePluginTags();
}

void LootDeployer::downloadList(std::string url, const std::string& file_name)
{
  const std::string tmp_file_name = file_name + ".tmp";
  std::ofstream fstream(dest_path_ / tmp_file_name, std::ios::binary);
  if(!fstream.is_open())
    throw std::runtime_error("Failed to update " + file_name + ": Could not write to: \"" +
                             dest_path_.string() + "\".");

  auto pos = url.find(" ");
  while(pos != std::string::npos)
  {
    url.replace(pos, 1, "%20");
    pos = url.find(" ");
  }
  cpr::Response response = cpr::Download(fstream, cpr::Url{ url });
  if(response.status_code != 200)
  {
    if(sfs::exists(dest_path_ / tmp_file_name))
      sfs::remove(dest_path_ / tmp_file_name);
    throw std::runtime_error("Could not download " + file_name + " from '" +
                             LIST_URLS.at(app_type_) + "'.\nTry to update the URL in the " +
                             "settings. Alternatively, you can manually download the " +
                             "file and place it in '" + dest_path_.string() +
                             "'. You can disable auto updates in '" +
                             (dest_path_ / CONFIG_FILE_NAME).string() + "'.");
  }
  if(sfs::exists(dest_path_ / file_name))
    sfs::remove(dest_path_ / file_name);
  sfs::rename(dest_path_ / tmp_file_name, dest_path_ / file_name);
}
