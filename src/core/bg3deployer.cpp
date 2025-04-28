#include "bg3deployer.h"
#include "pathutils.h"
#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <ranges>

namespace sfs = std::filesystem;
namespace str = std::ranges;
namespace pu = path_utils;


Bg3Deployer::Bg3Deployer(const std::filesystem::path& source_path,
                         const std::filesystem::path& dest_path,
                         const std::string& name) : PluginDeployer(source_path, dest_path, name)
{
  type_ = "Baldurs Gate 3 Deployer";
  is_autonomous_ = true;
  plugin_regex_ = R"(.*\.[pP][aA][kK])";
  plugin_file_line_regex_ = R"(^\s*(\*?)([^#]*)(\r?))";
  plugin_file_name_ = ".loadorder";
  config_file_name_ = ".pak_files.json";
  source_mods_file_name_ = ".plugin_mod_sources";

  if(!initPluginFile())
    loadPlugins();
  if(sfs::exists(dest_path_ / config_file_name_))
    loadSettingsPrivate();
  updatePluginsPrivate();
  cleanState();
  readSourceMods();
}

void Bg3Deployer::unDeploy(std::optional<ProgressNode*> progress_node)
{
  const std::string plugin_backup_path =
    dest_path_ / (plugin_file_name_ + UNDEPLOY_BACKUP_EXTENSION);
  if(!sfs::exists(plugin_backup_path))
    sfs::copy(
      dest_path_ / plugin_file_name_, plugin_backup_path, sfs::copy_options::overwrite_existing);

  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating plugins...", name_));
  plugins_.clear();
  uuid_map_.clear();
  pak_files_.clear();
  writePlugins();
  saveSettings();
}

std::vector<std::string> Bg3Deployer::getModNames() const
{
  std::vector<std::string> names{};
  names.reserve(plugins_.size());
  for(const auto& [uuid, enabled] : plugins_)
    names.push_back(pak_files_.at(uuid_map_.at(uuid)).getPluginName(uuid));
  return names;
}

void Bg3Deployer::setProfile(int profile)
{
  PluginDeployer::setProfile(profile);
  cleanState();
  writePlugins();
  saveSettings();
}

std::unordered_set<int> Bg3Deployer::getModConflicts(int mod_id,
                                                     std::optional<ProgressNode*> progress_node)
{
  std::unordered_set<int> conflicts{ mod_id };
  if(progress_node)
    (*progress_node)->setTotalSteps(plugins_.size());
  const std::string plugin_uuid = plugins_[mod_id].first;
  auto file = pak_files_[uuid_map_[plugin_uuid]];
  for(const auto& [i, pair] : str::enumerate_view(plugins_))
  {
    if(i == mod_id)
      continue;
    const auto& [uuid, enabled] = pair;
    if(file.conflictsWith(pak_files_[uuid_map_[uuid]]))
      conflicts.insert(i);
    if(progress_node)
      (*progress_node)->advance();
  }
  return conflicts;
}

void Bg3Deployer::updatePlugins()
{
  updatePluginsPrivate();
}

void Bg3Deployer::saveSettings() const
{
  saveSettingsPrivate();
}

void Bg3Deployer::loadSettings()
{
  loadSettingsPrivate();
}

void Bg3Deployer::resetSettings()
{
  num_profiles_ = 1;
  current_profile_ = 0;
  saveSettings();
}

void Bg3Deployer::cleanState()
{
  std::vector<int> plugins_to_remove;
  for(const auto& [i, pair] : str::enumerate_view(plugins_))
  {
    const auto& [uuid, enabled] = pair;
    if(!uuid_map_.contains(uuid) || !pak_files_.contains(uuid_map_[uuid]) ||
       !pak_files_[uuid_map_[uuid]].hasPlugin(uuid))
    {
      plugins_to_remove.push_back(i);
    }
  }
  for(int i : str::reverse_view(plugins_to_remove))
    plugins_.erase(plugins_.begin() + i);

  for(const auto& [path, file] : pak_files_)
  {
    for(const auto& plugin : file.getPlugins())
    {
      if(str::none_of(plugins_,
                      [&plugin](const auto& pair) { return pair.first == plugin.getUuid(); }))
      {
        plugins_.emplace_back(plugin.getUuid(), true);
        uuid_map_[plugin.getUuid()] = file.getSourceFile();
      }
    }
  }
}

void Bg3Deployer::writePlugins() const
{
  writePluginsPrivate();
}

bool Bg3Deployer::initPluginFile()
{
  const sfs::path plugin_file_path = dest_path_ / plugin_file_name_;
  if(sfs::exists(plugin_file_path))
    return false;

  const sfs::path bg3_plugin_file_path = dest_path_ / BG3_PLUGINS_FILE_NAME;
  pugi::xml_document xml_doc;
  xml_doc.load_file(bg3_plugin_file_path.c_str());
  plugins_.clear();

  pugi::xml_node order_node = xml_doc.child("save")
                                .find_child_by_attribute("id", "ModuleSettings")
                                .find_child_by_attribute("id", "root")
                                .child("children")
                                .find_child_by_attribute("id", "ModOrder")
                                .child("children");
  for(const auto& node : order_node.children())
  {
    pugi::xml_node attr = node.find_child_by_attribute("id", "UUID");
    const std::string uuid = attr.attribute("value").value();
    if(!Bg3Plugin::BG3_VANILLA_UUIDS.contains(uuid))
      plugins_.emplace_back(uuid, true);
  }

  PluginDeployer::writePlugins();
  return true;
}

void Bg3Deployer::updateSourceMods()
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
  for(const auto& [uuid, _] : plugins_)
  {
    auto iter = deployed_files.find(relative_path / uuid_map_[uuid]);
    if(iter != deployed_files.end())
      source_mods_[uuid] = iter->second;
  }
  writeSourceMods();
}

void Bg3Deployer::updatePluginTags() {}

void Bg3Deployer::updatePluginsPrivate()
{
  std::vector<sfs::path> pak_file_paths;
  for(const auto& dir_entry : sfs::directory_iterator(source_path_))
  {
    if(dir_entry.is_directory())
      continue;
    const std::string file_name = dir_entry.path().filename().string();
    if(std::regex_match(file_name, plugin_regex_) && !NON_PLUGIN_ARCHIVES.contains(file_name))
      pak_file_paths.push_back(pu::getRelativePath(dir_entry.path(), source_path_));
  }
  // remove missing files
  std::vector<std::filesystem::path> files_to_remove;
  for(const auto& [path, file] : pak_files_)
  {
    if(str::find(pak_file_paths, path) == pak_file_paths.end())
      files_to_remove.push_back(path);
  }
  for(const auto& path : files_to_remove)
  {
    std::vector<int> plugins_to_remove;
    for(const auto& plugin : pak_files_[path].getPlugins())
    {
      auto iter = str::find_if(
        plugins_, [&plugin](const auto& pair) { return pair.first == plugin.getUuid(); });
      if(iter != plugins_.end())
        plugins_to_remove.push_back(iter - plugins_.begin());
    }
    str::sort(plugins_to_remove);
    for(int i : str::reverse_view(plugins_to_remove))
      plugins_.erase(plugins_.begin() + i);
    pak_files_.erase(path);
  }
  // add new files and update modified files
  for(const auto& path : pak_file_paths)
  {
    if(pak_files_.contains(path))
    {
      if(pak_files_[path].timestampsMatch())
        continue;

      Bg3PakFile new_file;
      try
      {
        new_file = Bg3PakFile(path, source_path_);
      }
      catch(std::runtime_error& error)
      {
        log_(Log::LOG_WARNING,
             std::format("Failed to parse '{}':\n{}", path.string(), error.what()));
        continue;
      }
      catch(...)
      {
        log_(Log::LOG_WARNING, std::format("Failed to parse '{}'.", path.string()));
        continue;
      }

      std::vector<int> plugins_to_remove;
      for(const auto& old_plugin : pak_files_[path].getPlugins())
      {
        if(str::none_of(new_file.getPlugins(),
                        [&old_plugin](const auto& plugin)
                        { return old_plugin.getUuid() == plugin.getUuid(); }))
        {
          uuid_map_.erase(old_plugin.getUuid());
          auto iter = str::find_if(plugins_,
                                   [&old_plugin](const auto& pair)
                                   { return pair.first == old_plugin.getUuid(); });
          if(iter != plugins_.end())
            plugins_to_remove.push_back(iter - plugins_.begin());
        }
      }
      str::sort(plugins_to_remove);
      for(int i : str::reverse_view(plugins_to_remove))
        plugins_.erase(plugins_.begin() + i);

      for(const auto& new_plugin : new_file.getPlugins())
      {
        auto iter = str::find_if(
          plugins_, [&new_plugin](const auto& pair) { return pair.first == new_plugin.getUuid(); });
        if(iter == plugins_.end())
        {
          plugins_.emplace_back(new_plugin.getUuid(), true);
          uuid_map_[new_plugin.getUuid()] = path;
        }
      }
      pak_files_[path] = new_file;
    }
    else
    {
      Bg3PakFile new_file;
      try
      {
        new_file = Bg3PakFile(path, source_path_);
      }
      catch(std::runtime_error& error)
      {
        log_(Log::LOG_WARNING,
             std::format("Failed to parse '{}':\n{}", path.string(), error.what()));
        continue;
      }
      catch(...)
      {
        log_(Log::LOG_WARNING, std::format("Failed to parse '{}'.", path.string()));
        continue;
      }

      if(new_file.getPlugins().empty())
      {
        log_(Log::LOG_WARNING, std::format("Archive '{}' contains no plugins.", path.string()));
        continue;
      }
      pak_files_[path] = new_file;
      for(const auto& plugin : pak_files_[path].getPlugins())
      {
        auto iter = str::find_if(
          plugins_, [&plugin](const auto& pair) { return pair.first == plugin.getUuid(); });
        if(iter != plugins_.end())
        {
          if(!uuid_map_.contains(iter->first))
            uuid_map_[plugin.getUuid()] = path;
          else
          {
            log_(Log::LOG_WARNING,
                 std::format("Pak files '{}' and '{}' contain identical mods with UUID '{}'.\n",
                             path.filename().string(),
                             uuid_map_[iter->first].filename().string(),
                             iter->first) +
                   std::format("Ignoring version in '{}'.", path.filename().string()));
          }
        }
        else
        {
          plugins_.emplace_back(plugin.getUuid(), true);
          uuid_map_[plugin.getUuid()] = path;
        }
      }
    }
  }
  writePluginsPrivate();
  saveSettingsPrivate();
}

void Bg3Deployer::loadSettingsPrivate()
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
  pak_files_.clear();
  uuid_map_.clear();
  for(int i = 0; i < settings["pak_files"].size(); i++)
  {
    Bg3PakFile pak_file;
    try
    {
      pak_file = Bg3PakFile(settings["pak_files"][i], source_path_);
    }
    catch(std::runtime_error& error)
    {
      log_(Log::LOG_WARNING,
           std::format("Failed to parse '{}':\n{}", source_path_.string(), error.what()));
      continue;
    }
    catch(...)
    {
      log_(Log::LOG_WARNING, std::format("Failed to parse '{}'.", source_path_.string()));
      continue;
    }

    for(const auto& plugin : pak_file.getPlugins())
      uuid_map_[plugin.getUuid()] = pak_file.getSourceFile();
    pak_files_[pak_file.getSourceFile()] = pak_file;
  }
}

void Bg3Deployer::writePluginsPrivate() const
{
  PluginDeployer::writePlugins();

  const sfs::path plugin_file_path = dest_path_ / BG3_PLUGINS_FILE_NAME;
  pugi::xml_document xml_doc;
  xml_doc.load_file((dest_path_ / BG3_PLUGINS_FILE_NAME).c_str());
  pugi::xml_node xml_root = xml_doc.child("save")
                              .find_child_by_attribute("id", "ModuleSettings")
                              .find_child_by_attribute("id", "root")
                              .child("children");

  pugi::xml_node mods_node = xml_root.find_child_by_attribute("id", "Mods").child("children");
  std::vector<pugi::xml_node> nodes_to_remove;
  for(const auto& mod : mods_node.children())
  {
    pugi::xml_node attr = mod.find_child_by_attribute("id", "UUID");
    const std::string uuid = attr.attribute("value").value();
    if(!Bg3Plugin::BG3_VANILLA_UUIDS.contains(uuid))
      nodes_to_remove.push_back(mod);
  }
  for(const auto& node : nodes_to_remove)
    mods_node.remove_child(node);

  auto order_node = xml_root.find_child_by_attribute("id", "ModOrder");
  if(!order_node.empty())
    xml_root.remove_child(order_node);
  order_node = xml_root.append_child("node");
  order_node.append_attribute("id") = "ModOrder";
  order_node.append_child("children");
  auto order_child = order_node.first_child();

  for(const auto& [uuid, enabled] : plugins_)
  {
    if(!enabled)
      continue;

    const auto& plugins = pak_files_.at(uuid_map_.at(uuid)).getPlugins();
    auto iter =
      str::find_if(plugins, [uuid = uuid](const auto& plugin) { return plugin.getUuid() == uuid; });
    if(iter != plugins.end())
    {
      iter->addToXmlModsNode(mods_node);
      iter->addToXmlOrderNode(order_child);
    }
  }

  xml_doc.save_file(plugin_file_path.c_str());
}

void Bg3Deployer::saveSettingsPrivate() const
{
  Json::Value settings;
  settings["num_profiles"] = num_profiles_;
  settings["current_profile"] = current_profile_;
  for(const auto& [i, pair] : str::enumerate_view(pak_files_))
    settings["pak_files"][(int)i] = pair.second.toJson();
  sfs::path settings_file_path = dest_path_ / config_file_name_;
  std::ofstream file(settings_file_path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not write to \"" + settings_file_path.string() + "\".");
  file << settings;
  file.close();
}
