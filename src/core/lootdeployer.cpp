#include "lootdeployer.h"
#include "pathutils.h"
#include <chrono>
#include <cpr/cpr.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <regex>
#include <set>

namespace sfs = std::filesystem;
namespace str = std::ranges;
namespace pu = path_utils;


LootDeployer::LootDeployer(const sfs::path& source_path,
                           const sfs::path& dest_path,
                           const std::string& name,
                           bool init_tags,
                           bool perform_init) : PluginDeployer(source_path, dest_path, name)
{
  LIST_URLS = DEFAULT_LIST_URLS;
  // make sure no hard link related checks are performed
  deploy_mode_ = copy;
  enable_unsafe_sorting_ = true;
  if(!perform_init)
    return;
  type_ = "Loot Deployer";
  is_autonomous_ = true;
  plugin_regex_ = R"(.*\.[eE][sS][pPlLmM]$)";
  plugin_file_line_regex_ = R"(^\s*(\*?)([^#]*\.[eE][sS][pPlLmM])(\r?))";
  config_file_name_ = ".lmmconfig";
  tags_file_name_ = ".loot_tags";
  source_mods_file_name_ = ".lmm_mod_sources";
  updateAppType();
  setupPluginFiles();
  loadPlugins();
  updatePlugins();
  if(sfs::exists(dest_path_ / config_file_name_))
    loadSettingsPrivate();
  if(init_tags)
    readPluginTags();
  readSourceMods();
}

void LootDeployer::unDeploy(std::optional<ProgressNode*> progress_node)
{
  const std::string loadorder_backup_path =
    dest_path_ / ("." + app_plugin_file_name_ + UNDEPLOY_BACKUP_EXTENSION);
  const std::string plugin_backup_path =
    dest_path_ / ("." + plugin_file_name_ + UNDEPLOY_BACKUP_EXTENSION);
  if(pu::exists(loadorder_backup_path) && !pu::exists(plugin_backup_path))
    sfs::remove(loadorder_backup_path);
  else if(!pu::exists(loadorder_backup_path) && pu::exists(plugin_backup_path))
    sfs::remove(plugin_backup_path);
  else if(!pu::exists(loadorder_backup_path) && !pu::exists(plugin_backup_path))
  {
    sfs::copy(dest_path_ / app_plugin_file_name_, loadorder_backup_path);
    sfs::copy(dest_path_ / plugin_file_name_, plugin_backup_path);
  }

  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating plugins...", name_));
  updatePlugins();
  updatePluginTags();
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
    sfs::copy(dest_path_ / ("." + app_plugin_file_name_ + EXTENSION + std::to_string(source)),
              dest_path_ /
                ("." + app_plugin_file_name_ + EXTENSION + std::to_string(num_profiles_)));
  }
  else
  {
    sfs::copy(dest_path_ / plugin_file_name_,
              dest_path_ / ("." + plugin_file_name_ + EXTENSION + std::to_string(num_profiles_)));
    sfs::copy(dest_path_ / app_plugin_file_name_,
              dest_path_ /
                ("." + app_plugin_file_name_ + EXTENSION + std::to_string(num_profiles_)));
  }
  num_profiles_++;
  saveSettings();
}

void LootDeployer::removeProfile(int profile)
{
  if(profile >= num_profiles_ || profile < 0)
    return;
  std::string plugin_file = "." + plugin_file_name_ + EXTENSION + std::to_string(profile);
  std::string loadorder_file = "." + app_plugin_file_name_ + EXTENSION + std::to_string(profile);
  if(profile == current_profile_)
    setProfile(0);
  else if(profile < current_profile_)
    setProfile(current_profile_ - 1);
  sfs::remove(dest_path_ / plugin_file);
  sfs::remove(dest_path_ / loadorder_file);
  num_profiles_--;
  saveSettings();
}

void LootDeployer::setProfile(int profile)
{
  if(profile >= num_profiles_ || profile < 0 || profile == current_profile_)
    return;
  if(!sfs::exists(dest_path_ / plugin_file_name_) ||
     !sfs::exists(dest_path_ / app_plugin_file_name_) ||
     !sfs::exists(dest_path_ / ("." + plugin_file_name_ + EXTENSION + std::to_string(profile))) ||
     !sfs::exists(dest_path_ / ("." + app_plugin_file_name_ + EXTENSION + std::to_string(profile))))
  {
    resetSettings();
    return;
  }
  sfs::rename(dest_path_ / plugin_file_name_,
              dest_path_ /
                ("." + plugin_file_name_ + EXTENSION + std::to_string(current_profile_)));
  sfs::rename(dest_path_ / app_plugin_file_name_,
              dest_path_ /
                ("." + app_plugin_file_name_ + EXTENSION + std::to_string(current_profile_)));
  sfs::rename(dest_path_ / ("." + plugin_file_name_ + EXTENSION + std::to_string(profile)),
              dest_path_ / plugin_file_name_);
  sfs::rename(dest_path_ / ("." + app_plugin_file_name_ + EXTENSION + std::to_string(profile)),
              dest_path_ / app_plugin_file_name_);
  current_profile_ = profile;
  saveSettings();
  loadPlugins();
  updatePlugins();
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
                             (dest_path_ / config_file_name_).string() + "'.");
  auto loot_handle = loot::CreateGameHandle(app_type_, source_path_, dest_path_);
  sfs::path user_list_path(dest_path_ / "userlist.yaml");
  if(!sfs::exists(user_list_path))
    user_list_path = "";
  sfs::path prelude_path(dest_path_ / "prelude.yaml");
  if(!sfs::exists(prelude_path))
    prelude_path = "";
  loot_handle->GetDatabase().LoadMasterlistWithPrelude(master_list_path, prelude_path);
  loot_handle->GetDatabase().LoadUserlist(user_list_path);
  if(progress_node)
    (*progress_node)->child(1).advance();

  std::vector<sfs::path> plugin_paths;
  std::vector<std::string> plugin_file_names;
  plugin_paths.reserve(plugins_.size());
  plugin_file_names.reserve(plugins_.size());
  for(const auto& [path, s] : plugins_)
  {
    plugin_paths.emplace_back(source_path_ / path);
    plugin_file_names.emplace_back(path);
  }
  loot_handle->LoadPlugins(plugin_paths, false);
  auto sorted_plugins = loot_handle->SortPlugins(plugin_file_names);
  if(progress_node)
    (*progress_node)->child(2).advance();

  std::vector<std::pair<std::string, bool>> new_plugins;
  new_plugins.reserve(plugins_.size());
  std::set<std::string> conflicting;
  int num_light_plugins = 0;
  int num_master_plugins = 0;
  int num_standard_plugins = 0;
  tags_.clear();
  for(const auto& plugin : sorted_plugins)
  {
    auto iter = str::find_if(plugins_, [plugin](const auto& p) { return p.first == plugin; });
    bool enabled = true;
    if(iter != plugins_.end())
      enabled = iter->second;
    const auto cur_plugin = loot_handle->GetPlugin(plugin);
    if(cur_plugin->IsLightPlugin())
    {
      num_light_plugins++;
      tags_.push_back({ LIGHT_PLUGIN });
    }
    else if(cur_plugin->IsMaster())
    {
      num_master_plugins++;
      tags_.push_back({ MASTER_PLUGIN });
    }
    else
    {
      num_standard_plugins++;
      tags_.push_back({ STANDARD_PLUGIN });
    }
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
  log_(Log::LOG_DEBUG, std::format("LOOT: App type {}", static_cast<int>(app_type_)));
  log_(Log::LOG_INFO,
       std::format("LOOT: Total Plugins: {}, Master: {}, Standard: {}, Light: {}",
                   new_plugins.size(),
                   num_master_plugins,
                   num_standard_plugins,
                   num_light_plugins));
  if(enable_unsafe_sorting_)
    plugins_ = new_plugins;
  writePluginTags();
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
      dest_path_ / ("." + app_plugin_file_name_ + EXTENSION + std::to_string(i));
    sfs::remove(plugin_path);
    sfs::remove(load_order_path);
  }
  current_profile_ = 0;
  num_profiles_ = 1;
  sfs::remove(dest_path_ / config_file_name_);
}

std::map<std::string, int> LootDeployer::getAutoTagMap()
{
  return { { LIGHT_PLUGIN, num_light_plugins_ },
           { MASTER_PLUGIN, num_master_plugins_ },
           { STANDARD_PLUGIN, num_standard_plugins_ } };
}

void LootDeployer::writePlugins() const
{
  PluginDeployer::writePlugins();

  std::ofstream plugins_file;
  plugins_file.open(dest_path_ / app_plugin_file_name_);
  if(!plugins_file.is_open())
    throw std::runtime_error("Could not open " + app_plugin_file_name_ + "!");
  for(const auto& [name, enabled] : plugins_)
  {
    if(enabled)
      plugins_file << name << "\n";
  }
  plugins_file.close();

  if(APP_TYPE_WITH_FILE_MOD_ORDER.contains(app_type_))
  {
    for(const auto& [i, pair] : str::enumerate_view(plugins_))
    {
      const auto& [name, enabled] = pair;
      std::tm tm = { 0, static_cast<int>(i), 0, 1, 0, 100 };
      tm.tm_isdst = -1;
      std::filesystem::file_time_type time_point =
        std::chrono::file_clock::from_sys(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
      const sfs::path plugin_path = source_path_ / name;
      if(sfs::exists(plugin_path))
      {
        sfs::last_write_time(plugin_path, time_point);
        if(sfs::is_symlink(plugin_path))
        {
          const sfs::path actual_path = sfs::read_symlink(plugin_path);
          sfs::last_write_time(actual_path, time_point);
        }
      }
    }
  }
}

void LootDeployer::saveSettings() const
{
  Json::Value settings;
  settings["num_profiles"] = num_profiles_;
  settings["current_profile"] = current_profile_;
  settings["list_download_time"] = list_download_time_;
  settings["auto_update_master_list"] = auto_update_lists_;
  sfs::path settings_file_path = dest_path_ / config_file_name_;
  std::ofstream file(settings_file_path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not write to \"" + settings_file_path.string() + "\".");
  file << settings;
  file.close();
}

void LootDeployer::loadSettings()
{
  loadSettingsPrivate();
}

void LootDeployer::updateAppType()
{
  for(const auto& [type, file] : TYPE_IDENTIFIERS)
  {
    if(pu::pathExists(file, source_path_))
    {
      app_type_ = type;
      if(APP_TYPE_WITH_FILE_MOD_ORDER.contains(type))
      {
        app_plugin_file_name_ = PLUGIN_FILE_NAMES.at(type);
        plugin_file_name_ = LOADORDER_FILE_NAME;
      }
      else
      {
        plugin_file_name_ = PLUGIN_FILE_NAMES.at(type);
        app_plugin_file_name_ = LOADORDER_FILE_NAME;
      }
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
  const std::chrono::time_point<std::chrono::system_clock> update_time{
    std::chrono::seconds(list_download_time_)};
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
  resetSettingsPrivate();
}

void LootDeployer::setupPluginFiles()
{
  if(sfs::exists(dest_path_ / plugin_file_name_) && sfs::exists(dest_path_ / app_plugin_file_name_))
    return;
  updatePlugins();
}

void LootDeployer::updatePluginTags()
{
  updatePluginTagsPrivate();
}

void LootDeployer::readPluginTags()
{
  const sfs::path tag_file_path = dest_path_ / tags_file_name_;
  if(!sfs::exists(tag_file_path))
  {
    updatePluginTagsPrivate();
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
    updatePluginTagsPrivate();
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
    sfs::remove(dest_path_ / tmp_file_name);
    throw std::runtime_error("Could not download " + file_name + " from '" +
                             LIST_URLS.at(app_type_) + "'.\nTry to update the URL in the " +
                             "settings. Alternatively, you can manually download the " +
                             "file and place it in '" + dest_path_.string() +
                             "'. You can disable auto updates in '" +
                             (dest_path_ / config_file_name_).string() + "'.");
  }
  sfs::remove(dest_path_ / file_name);
  sfs::rename(dest_path_ / tmp_file_name, dest_path_ / file_name);
}

void LootDeployer::restoreUndeployBackupIfExists()
{
  const std::string loadorder_backup_path =
    dest_path_ / ("." + app_plugin_file_name_ + UNDEPLOY_BACKUP_EXTENSION);
  const std::string plugin_backup_path =
    dest_path_ / ("." + plugin_file_name_ + UNDEPLOY_BACKUP_EXTENSION);
  if(pu::exists(loadorder_backup_path) && !pu::exists(plugin_backup_path))
    sfs::remove(loadorder_backup_path);
  else if(!pu::exists(loadorder_backup_path) && pu::exists(plugin_backup_path))
    sfs::remove(plugin_backup_path);
  else if(sfs::exists(loadorder_backup_path) && sfs::exists(plugin_backup_path))
  {
    log_(Log::LOG_DEBUG, std::format("Deployer '{}': Restoring undeploy backup.", name_));
    sfs::remove(dest_path_ / app_plugin_file_name_);
    sfs::rename(loadorder_backup_path, dest_path_ / app_plugin_file_name_);
    sfs::remove(dest_path_ / plugin_file_name_);
    sfs::rename(plugin_backup_path, dest_path_ / plugin_file_name_);
    loadPlugins();
  }
}

void LootDeployer::loadSettingsPrivate()
{
  Json::Value settings;
  sfs::path settings_file_path = dest_path_ / config_file_name_;
  if(!sfs::exists(settings_file_path))
  {
    resetSettingsPrivate();
    return;
  }
  std::ifstream file(settings_file_path, std::fstream::binary);
  if(!file.is_open())
  {
    resetSettingsPrivate();
    return;
  }
  file >> settings;
  file.close();
  if(!settings.isMember("num_profiles") || !settings.isMember("current_profile") ||
     !settings.isMember("list_download_time") || !settings.isMember("auto_update_master_list"))
  {
    resetSettingsPrivate();
    return;
  }
  num_profiles_ = settings["num_profiles"].asInt();
  current_profile_ = settings["current_profile"].asInt();
  list_download_time_ = settings["list_download_time"].asInt64();
  auto_update_lists_ = settings["auto_update_master_list"].asBool();
}

void LootDeployer::resetSettingsPrivate()
{
  num_profiles_ = 1;
  current_profile_ = 0;
  auto_update_lists_ = true;
  list_download_time_ = 0;
}

void LootDeployer::updatePluginTagsPrivate()
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
