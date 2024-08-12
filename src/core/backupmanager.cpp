#include "backupmanager.h"
#include "parseerror.h"
#include "pathutils.h"
#include <format>
#include <fstream>

namespace sfs = std::filesystem;
namespace pu = path_utils;


void BackupManager::addTarget(const sfs::path& path,
                              const std::string& name,
                              const std::vector<std::string>& backup_names)
{
  if(!sfs::exists(path))
    throw std::runtime_error(std::format("Path \"{}\" does not exist", path.string()));
  sfs::path trimmed_path = path;
  if(path.string().ends_with(sfs::path::preferred_separator))
    trimmed_path = path.string().erase(path.string().size() - 1, 1);
  for(const auto& target : targets_)
  {
    if(target.path == trimmed_path)
      throw std::runtime_error(std::format(
        "\"{}\" is already managed as \"{}\" by BackupManager", path.string(), target.target_name));
  }
  if(sfs::exists(getConfigPath(trimmed_path)))
    addTarget(trimmed_path);
  else
  {
    if(backup_names.empty())
      throw std::runtime_error("At least one backup name must be provided");
    targets_.emplace_back(trimmed_path,
                          name,
                          std::vector<std::string>{ backup_names[0] },
                          std::vector<int>(num_profiles_, 0));
    for(int i = 1; i < backup_names.size(); i++)
      addBackup(targets_.size() - 1, backup_names[i]);
  }
  updateSettings();
}

void BackupManager::addTarget(const sfs::path& path)
{
  if(!sfs::exists(path))
    throw std::runtime_error(std::format("Path \"{}\" does not exist", path.string()));
  if(!sfs::exists(getConfigPath(path)))
    throw std::runtime_error(
      std::format("Could not find settings file at \"{}\"", getConfigPath(path).string()));
  for(const auto& target : targets_)
  {
    if(target.path == path)
      throw std::runtime_error(std::format(
        "\"{}\" is already managed as \"{}\" by BackupManager", path.string(), target.target_name));
  }
  targets_.push_back({ path, "", {}, {} });
  updateState();
}

void BackupManager::addBackup(int target_id, const std::string& name, int source)
{
  if(target_id < 0 || target_id >= targets_.size())
    throw std::runtime_error(std::format("Invalid target id: {}", target_id));
  updateDirectories(target_id);
  auto& target = targets_[target_id];
  sfs::path source_path;
  if(source >= 0 && source < targets_[target_id].backup_names.size())
    source_path = getBackupPath(target_id, source);
  else
    source_path = getBackupPath(target_id, target.active_members[cur_profile_]);
  sfs::copy(source_path,
            getBackupPath(target.path, target.backup_names.size()),
            sfs::copy_options::recursive | sfs::copy_options::copy_symlinks);
  target.backup_names.push_back(name);
  updateSettings();
}

void BackupManager::removeTarget(int target_id)
{
  for(int backup = 0; backup < targets_[target_id].backup_names.size(); backup++)
  {
    if(backup == targets_[target_id].active_members[cur_profile_])
      continue;
    const auto path = getBackupPath(target_id, backup);
    if(sfs::exists(path))
      sfs::remove_all(path);
  }
  const auto config_file = getConfigPath(targets_[target_id].path);
  if(sfs::exists(config_file))
    sfs::remove(config_file);
  targets_.erase(targets_.begin() + target_id);
}

void BackupManager::removeBackup(int target_id, int backup_id, bool update_dirs)
{
  if(target_id < 0 || target_id >= targets_.size())
    throw std::runtime_error(std::format("Invalid target id: {}", target_id));
  if(update_dirs)
    updateDirectories(target_id);
  if(targets_[target_id].backup_names.size() == 1)
    throw std::runtime_error(
      std::format("No backups to remove for \"{}\"", targets_[target_id].target_name));
  auto& target = targets_[target_id];
  if(backup_id < 0 || backup_id >= target.backup_names.size())
    throw std::runtime_error(
      std::format("Invalid backup id: {} for target: {}", backup_id, target_id));

  if(target.active_members[cur_profile_] == backup_id)
    setActiveBackup(target_id, backup_id == 0 ? 1 : 0);
  for(int prof = 0; prof < num_profiles_; prof++)
    target.active_members[prof] = 0;
  sfs::path backup_path = getBackupPath(target.path, backup_id);
  if(sfs::exists(backup_path))
    sfs::remove_all(backup_path);
  for(int i = backup_id + 1; i < target.backup_names.size(); i++)
  {
    sfs::path cur_path = getBackupPath(target.path, i);
    if(sfs::exists(cur_path))
      sfs::rename(cur_path, getBackupPath(target.path, i - 1));
  }
  target.backup_names.erase(target.backup_names.begin() + backup_id);
  if(update_dirs)
    updateSettings();
}

void BackupManager::setActiveBackup(int target_id, int backup_id)
{
  if(target_id < 0 || target_id >= targets_.size())
    throw std::runtime_error(std::format("Invalid target id: {}", target_id));
  updateDirectories(target_id);
  auto& target = targets_[target_id];
  if(backup_id < 0 || backup_id >= target.backup_names.size())
    throw std::runtime_error(
      std::format("Invalid backup id: {} for target: \"{}\"", target.target_name, backup_id));
  int active_id = target.active_members[cur_profile_];
  if(backup_id == active_id)
    return;
  sfs::rename(target.path, getBackupPath(target.path, active_id));
  sfs::rename(getBackupPath(target.path, backup_id), target.path);
  target.active_members[cur_profile_] = backup_id;
  target.cur_active_member = backup_id;
  updateSettings();
}

void BackupManager::setProfile(int profile)
{
  if(profile == cur_profile_)
    return;
  for(int target_id = 0; target_id < targets_.size(); target_id++)
  {
    auto& target = targets_[target_id];
    int old_id = target.active_members[cur_profile_];
    int new_id = target.active_members[profile];
    if(old_id == new_id)
      continue;
    setActiveBackup(target_id, new_id);
    target.active_members[cur_profile_] = old_id;
  }
  cur_profile_ = profile;
}

void BackupManager::addProfile(int source)
{
  num_profiles_++;
  if(cur_profile_ < 0 || cur_profile_ >= num_profiles_)
    cur_profile_ = 0;
  for(auto& target : targets_)
  {
    int active_id = source >= 0 && source < num_profiles_ ? target.active_members[source] : 0;
    target.active_members.push_back(active_id);
  }
  updateSettings();
}

void BackupManager::removeProfile(int profile)
{
  num_profiles_--;
  for(auto& target : targets_)
    target.active_members.erase(target.active_members.begin() + profile);
  if(profile == cur_profile_)
    setProfile(0);
  else if(cur_profile_ > profile)
    cur_profile_--;
  updateSettings();
}

std::vector<BackupTarget> BackupManager::getTargets() const
{
  auto ret_targets = targets_;
  for(auto& target : ret_targets)
    target.cur_active_member = target.active_members[cur_profile_];
  return ret_targets;
}

void BackupManager::reset()
{
  targets_.clear();
  num_profiles_ = 0;
}

int BackupManager::getNumTargets()
{
  return targets_.size();
}

int BackupManager::getNumBackups(int target_id)
{
  return targets_[target_id].backup_names.size();
}

void BackupManager::setBackupName(int target_id, int backup_id, const std::string& name)
{
  targets_[target_id].backup_names[backup_id] = name;
  updateSettings();
}

void BackupManager::setBackupTargetName(int target_id, const std::string& name)
{
  targets_[target_id].target_name = name;
  updateSettings();
}

void BackupManager::overwriteBackup(int target_id, int source_backup, int dest_backup)
{
  if(source_backup == dest_backup)
    return;
  const auto source_path = getBackupPath(target_id, source_backup);
  const auto dest_path = getBackupPath(target_id, dest_backup);
  sfs::remove_all(dest_path);
  sfs::copy(
    source_path, dest_path, sfs::copy_options::recursive | sfs::copy_options::overwrite_existing);
}

void BackupManager::setLog(const std::function<void(Log::LogLevel, const std::string&)>& new_log)
{
  log_ = new_log;
}

void BackupManager::updateDirectories(int target_id)
{
  std::vector<int> missing_dirs;
  for(int backup_id = 0; backup_id < targets_[target_id].backup_names.size(); backup_id++)
  {
    if(!sfs::exists(getBackupPath(targets_[target_id].path, backup_id)) &&
       backup_id != targets_[target_id].active_members[cur_profile_])
      missing_dirs.push_back(backup_id);
  }

  for(int j = missing_dirs.size() - 1; j >= 0; j--)
  {
    log_(Log::LOG_WARNING,
         std::format("Could not find backup \"{}\" for target \"{}\".",
                     targets_[target_id].backup_names[missing_dirs[j]],
                     targets_[target_id].target_name));
    removeBackup(target_id, missing_dirs[j], false);
  }

  std::vector<sfs::path> extra_dirs;
  for(const auto& dir_entry : sfs::directory_iterator(targets_[target_id].path.parent_path()))
  {
    const auto file_name = dir_entry.path().filename();
    if(!file_name.has_extension() || file_name.extension().string() != BAK_EXTENSION)
      continue;
    if(!file_name.stem().has_extension())
      continue;
    std::string extension = file_name.stem().extension();
    if(sfs::path(file_name).stem().stem() != targets_[target_id].path.filename())
      continue;
    if(extension.starts_with("."))
      extension.replace(0, 1, "");
    if(extension.find_first_not_of("0123456789") != extension.npos)
      continue;
    int id = std::stoi(extension);
    if(id >= targets_[target_id].backup_names.size() ||
       id == targets_[target_id].active_members[cur_profile_])
      extra_dirs.push_back(dir_entry.path());
  }

  for(const auto& path : extra_dirs)
  {
    sfs::path new_path = path.string() + "OLD";
    int i = 0;
    while(sfs::exists(new_path))
      new_path = path.string() + "OLD" + std::to_string(i++);
    log_(Log::LOG_WARNING,
         std::format(
           "Unknown backup found at \"{}\". Moving to \"{}\".", path.string(), new_path.string()));
    sfs::rename(path, new_path);
  }
  updateSettings();
}

void BackupManager::updateDirectories()
{
  for(int target_id = 0; target_id < targets_.size(); target_id++)
    updateDirectories(target_id);
  updateSettings();
}

void BackupManager::updateState()
{
  for(auto& target : targets_)
  {
    const auto settings = readSettings(getConfigPath(target.path));
    auto keys = { "path", "target_name", "backup_names", "active_members" };
    for(const auto& key : keys)
    {
      if(!settings.isMember(key))
        throw ParseError(std::format("\"{}\" is missing in \"{}\"", key, target.path.string()));
    }
    if(settings["path"].asString() != target.path.string())
      throw ParseError(std::format(
        "Invalid path \"{}\" in \"{}\"", settings["path"].asString(), target.path.string()));
    std::vector<std::string> new_names;
    auto names = settings["backup_names"];
    if(names.empty())
      throw ParseError(std::format("No backups found for \"{}\"", target.path.string()));
    for(int i = 0; i < names.size(); i++)
      new_names.push_back(names[i].asString());
    std::vector<int> new_active_members;
    auto active_members = settings["active_members"];
    for(int i = 0; i < active_members.size(); i++)
    {
      int member = active_members[i].asInt();
      if(member < 0 || member >= new_names.size())
        throw ParseError(
          std::format("Invalid active member\"{}\" in \"{}\"", member, target.path.string()));
      new_active_members.push_back(member);
    }
    if(active_members.size() != num_profiles_)
      throw ParseError(
        std::format("Failed to parse active_members in \"{}\"", target.path.string()));
    target.target_name = settings["target_name"].asString();
    target.backup_names = new_names;
    target.active_members = new_active_members;
  }
  updateDirectories();
}

void BackupManager::updateSettings()
{
  for(const auto& target : targets_)
  {
    Json::Value settings;
    settings["path"] = target.path.string();
    settings["target_name"] = target.target_name;
    for(int i = 0; i < target.backup_names.size(); i++)
      settings["backup_names"][i] = target.backup_names[i];
    for(int i = 0; i < target.active_members.size(); i++)
      settings["active_members"][i] = target.active_members[i];
    writeSettings(getConfigPath(target.path), settings);
  }
}

void BackupManager::writeSettings(const sfs::path& path, const Json::Value& settings) const
{
  std::ofstream file(path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not write to \"" + path.string() + "\".");
  file << settings;
  file.close();
}

Json::Value BackupManager::readSettings(const sfs::path& path) const
{
  Json::Value settings;
  std::ifstream file(path, std::fstream::binary);
  if(!file.is_open())
    throw std::runtime_error("Error: Could not read from \"" + path.string() + "\".");
  file >> settings;
  file.close();
  return settings;
}

sfs::path BackupManager::getConfigPath(const sfs::path& path) const
{
  if(!path.has_parent_path())
    throw std::runtime_error("Creating backups of the filesystem root is not supported");

  sfs::path dest = path;
  if(path.string().ends_with("/"))
    dest = dest.parent_path();
  return dest.parent_path() /
         ("." + pu::getRelativePath(dest, dest.parent_path()) + JSON_EXTENSION);
}

sfs::path BackupManager::getBackupPath(const sfs::path& path, int backup) const
{
  return path.string() + "." + std::to_string(backup) + BAK_EXTENSION;
}

sfs::path BackupManager::getBackupPath(int target, int backup) const
{
  sfs::path file_path = targets_[target].path;
  if(targets_[target].active_members[cur_profile_] == backup)
    return file_path;
  return getBackupPath(file_path, backup);
}
