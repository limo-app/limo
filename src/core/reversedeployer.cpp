#include "reversedeployer.h"
#include "pathutils.h"
#include "json/json.h"
#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>

namespace sfs = std::filesystem;
namespace pu = path_utils;
namespace str = std::ranges;


ReverseDeployer::ReverseDeployer(const sfs::path& source_path,
                                 const sfs::path& dest_path,
                                 const std::string& name,
                                 DeployMode deploy_mode,
                                 bool separate_profile_dirs,
                                 bool update_ignore_list) :
  Deployer(source_path, dest_path, name, deploy_mode), separate_profile_dirs_(separate_profile_dirs)
{
  type_ = "Reverse Deployer";
  is_autonomous_ = true;
  if(sfs::exists(source_path_ / managed_files_name_))
    readManagedFiles();
  else
    writeManagedFiles();
  if(sfs::exists(dest_path_ / ignore_list_file_name_))
    readIgnoredFiles();
  else if(update_ignore_list)
    updateIgnoredFiles(true);
  else
    writeIgnoredFiles();
}

void ReverseDeployer::updateManagedFiles(bool write)
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating managed files...", name_));
  updateFilesInDir(dest_path_, {}, dest_path_);
  updateCurrentLoadorder();
  moveFilesFromTargetToSource();
  if(write)
    writeManagedFiles();
}

std::map<int, unsigned long> ReverseDeployer::deploy(std::optional<ProgressNode*> progress_node)
{
  if(deployed_profile_ != current_profile_ && deployed_profile_ > -1)
  {
    const int cur_profile = current_profile_;
    current_profile_ = deployed_profile_;
    if(separate_profile_dirs_)
      updateManagedFiles(false);
    unDeploy();
    current_profile_ = cur_profile;
  }
  updateManagedFiles();
  deployManagedFiles();
  writeManagedFiles();
  return {};
}

std::map<int, unsigned long> ReverseDeployer::deploy(const std::vector<int>& loadorder,
                                                     std::optional<ProgressNode*> progress_node)
{
  return deploy(progress_node);
}

void ReverseDeployer::unDeploy(std::optional<ProgressNode*> progress_node)
{
  for(const auto& [path, _] : current_loadorder_)
  {
    const sfs::path full_dest_path = dest_path_ / path;
    if(sfs::exists(full_dest_path))
      sfs::remove(full_dest_path);
  }
  deployed_profile_ = -1;
}

void ReverseDeployer::changeLoadorder(int from_index, int to_index)
{
  log_(Log::LOG_DEBUG,
       "WARNING: You are trying to change the load order of a reverse deployer."
       "This will have no effect.");
}

void ReverseDeployer::setModStatus(int mod_id, bool status)
{
  if(mod_id >= current_loadorder_.size() || mod_id < 0)
    return;

  current_loadorder_[mod_id].second = status;
  managed_files_[current_profile_][current_loadorder_[mod_id].first] = status;
  writeManagedFiles();
}

std::vector<std::vector<int>> ReverseDeployer::getConflictGroups() const
{
  std::vector<int> group(managed_files_.size());
  str::iota(group.begin(), group.end(), 0);
  return { group };
}

std::vector<std::string> ReverseDeployer::getModNames() const
{
  std::vector<std::string> names;
  names.reserve(current_loadorder_.size());
  for(const auto& [path, enabled] : current_loadorder_)
    names.push_back(path.string());
  return names;
}

void ReverseDeployer::addProfile(int source)
{
  managed_files_.push_back({});
  if(source != -1)
  {
    managed_files_.back() = managed_files_[source];
    if(separate_profile_dirs_)
      sfs::create_directories(source_path_ / std::to_string(managed_files_.size() - 1));
  }
  writeManagedFiles();
}

void ReverseDeployer::removeProfile(int profile)
{
  if(separate_profile_dirs_)
  {
    if(profile == deployed_profile_)
    {
      int cur_profile = current_profile_;
      current_profile_ = profile;
      unDeploy();
      current_profile_ = cur_profile;
    }
    if(sfs::exists(source_path_ / std::to_string(profile)))
      sfs::remove_all(source_path_ / std::to_string(profile));
    for(int prof = profile + 1; prof < managed_files_.size() - 1; prof++)
    {
      if(sfs::exists(source_path_ / std::to_string(prof)))
        sfs::rename(source_path_ / std::to_string(prof), source_path_ / std::to_string(prof - 1));
    }
  }

  managed_files_.erase(managed_files_.begin() + profile);
  if(profile == current_profile_)
  {
    current_profile_ = 0;
    updateCurrentLoadorder();
    if(separate_profile_dirs_)
      deployManagedFiles();
  }
  else if(profile < current_profile_)
  {
    current_profile_--;
  }
  writeManagedFiles();
}

void ReverseDeployer::setProfile(int profile)
{
  if(profile == current_profile_)
    return;
  current_profile_ = profile;
  updateCurrentLoadorder();
}

void ReverseDeployer::setConflictGroups(const std::vector<std::vector<int>>& newConflict_groups) {}

int ReverseDeployer::getNumMods() const
{
  return current_loadorder_.size();
}

std::vector<std::tuple<int, bool>> ReverseDeployer::getLoadorder() const
{
  std::vector<std::tuple<int, bool>> loadorder;
  loadorder.reserve(current_loadorder_.size());
  for(const auto& [i, enabled] : str::enumerate_view(current_loadorder_ | std::views::values))
    loadorder.emplace_back(i, enabled);
  return loadorder;
}

bool ReverseDeployer::addMod(int mod_id, bool enabled, bool update_conflicts)
{
  log_(Log::LOG_DEBUG,
       "WARNING: You are trying to add a mod to an autonomous"
       " deployer. This will have no effect.");
  return false;
}

bool ReverseDeployer::removeMod(int mod_id)
{
  log_(Log::LOG_DEBUG,
       "WARNING: You are trying to remove a mod from an autonomous"
       " deployer. This will have no effect.");
  return false;
}

bool ReverseDeployer::hasMod(int mod_id) const
{
  return false;
}

bool ReverseDeployer::swapMod(int old_id, int new_id)
{
  log_(Log::LOG_DEBUG,
       "WARNING: You are trying to swap a mod in an autonomous"
       " deployer. This will have no effect.");
  return false;
}

std::vector<ConflictInfo> ReverseDeployer::getFileConflicts(
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

std::unordered_set<int> ReverseDeployer::getModConflicts(int mod_id,
                                                         std::optional<ProgressNode*> progress_node)
{
  if(progress_node)
  {
    (*progress_node)->setTotalSteps(1);
    (*progress_node)->advance();
  }
  return {};
}

void ReverseDeployer::sortModsByConflicts(std::optional<ProgressNode*> progress_node)
{
  if(progress_node)
  {
    (*progress_node)->setTotalSteps(1);
    (*progress_node)->advance();
  }
}

void ReverseDeployer::cleanup()
{
  if(deployed_profile_ != -1)
    current_profile_ = deployed_profile_;
  unDeploy();
}

std::optional<bool> ReverseDeployer::getModStatus(int mod_id)
{
  return {};
}

std::vector<std::vector<std::string>> ReverseDeployer::getAutoTags()
{
  return {};
}

std::map<std::string, int> ReverseDeployer::getAutoTagMap()
{
  return {};
}

std::vector<std::pair<std::filesystem::__cxx11::path, int>>
ReverseDeployer::getExternallyModifiedFiles(std::optional<ProgressNode*> progress_node) const
{
  if(progress_node)
  {
    (*progress_node)->setTotalSteps(1);
    (*progress_node)->advance();
  }
  return {};
}

void ReverseDeployer::keepOrRevertFileModifications(const FileChangeChoices& changes_to_keep) const
{}

void ReverseDeployer::updateDeployedFilesForMod(int mod_id,
                                                std::optional<ProgressNode*> progress_node) const
{
  if(progress_node)
  {
    (*progress_node)->setTotalSteps(1);
    (*progress_node)->advance();
  }
}

void ReverseDeployer::updateIgnoredFiles(bool write)
{
  log_(Log::LOG_DEBUG, std::format("Deployer {}: Updating ignored files...", name_));
  ignored_files_.clear();
  updateFilesInDir(dest_path_, {}, {}, true);
  if(write)
    writeIgnoredFiles();
}

void ReverseDeployer::deleteIgnoredFiles()
{
  ignored_files_.clear();
  writeIgnoredFiles();
}

std::vector<std::string> ReverseDeployer::getIgnoredFiles() const
{
  std::vector<std::string> ignored_files_vector;
  ignored_files_vector.reserve(ignored_files_.size());
  for(const auto& file : ignored_files_)
    ignored_files_vector.push_back(file);
  return ignored_files_vector;
}

void ReverseDeployer::enableSeparateDirs(bool enabled)
{
  if(enabled == separate_profile_dirs_)
    return;

  if(enabled)
  {
    int temp_id = 0;
    sfs::path temp_path = source_path_ / ("rev_depl_temp_dir_" + std::to_string(temp_id));
    while(sfs::exists(temp_path))
      temp_path = source_path_ / ("rev_depl_temp_dir_" + std::to_string(++temp_id));
    sfs::create_directories(temp_path);
    for(const auto& dir_entry : sfs::directory_iterator(source_path_))
    {
      if(dir_entry.path() != temp_path &&
         dir_entry.path().string() != (source_path_ / managed_files_name_).string())
      {
        const std::string relative_path = pu::getRelativePath(dir_entry.path(), source_path_);
        sfs::rename(dir_entry.path(), source_path_ / temp_path / relative_path);
      }
    }
    sfs::rename(temp_path, source_path_ / std::to_string(current_profile_));
    for(int prof = 0; prof < managed_files_.size(); prof++)
    {
      if(prof != current_profile_)
      {
        sfs::create_directories(source_path_ / std::to_string(prof));
        managed_files_[prof].clear();
      }
    }
  }
  else
  {
    log_(Log::LOG_INFO, std::format("Deployer {}: Deleting files for inactive profiles...", name_));
    for(int prof = 0; prof < managed_files_.size(); prof++)
    {
      if(prof != current_profile_ && sfs::exists(source_path_ / std::to_string(prof)))
        sfs::remove_all(source_path_ / std::to_string(prof));
    }
    int temp_id = 0;
    sfs::path temp_dir = "rev_depl_temp_dir_" + std::to_string(temp_id);
    while(sfs::exists(source_path_ / std::to_string(current_profile_) / temp_dir))
      temp_dir = "rev_depl_temp_dir_" + std::to_string(++temp_id);
    sfs::rename(source_path_ / std::to_string(current_profile_), source_path_ / temp_dir);
    for(const auto& dir_entry : sfs::directory_iterator(source_path_ / temp_dir))
    {
      std::string relative_path = pu::getRelativePath(dir_entry.path(), source_path_ / temp_dir);
      sfs::rename(dir_entry.path(), source_path_ / relative_path);
    }
    sfs::remove_all(source_path_ / temp_dir);
  }
  writeManagedFiles();
}

bool ReverseDeployer::usesSeparateDirs() const
{
  return separate_profile_dirs_;
}

int ReverseDeployer::getNumIgnoredFiles() const
{
  return ignored_files_.size();
}

int ReverseDeployer::getNumProfiles() const
{
  return managed_files_.size();
}

void ReverseDeployer::readIgnoredFiles()
{
  ignored_files_.clear();
  const sfs::path ignored_list_path = dest_path_ / ignore_list_file_name_;
  std::ifstream file(ignored_list_path, std::ios::binary);
  if(!file.is_open())
    throw std::runtime_error("Could not read \"" + ignored_list_path.string() + "\".");
  Json::Value json_object;
  file >> json_object;
  for(int i = 0; i < json_object["ignored_files"].size(); i++)
    ignored_files_.insert(json_object["ignored_files"][i].asString());
}

void ReverseDeployer::writeIgnoredFiles() const
{
  Json::Value json_object;
  for(const auto& [i, file] : str::enumerate_view(ignored_files_))
    json_object["ignored_files"][(int)i] = file;

  const sfs::path ignored_list_path = dest_path_ / ignore_list_file_name_;
  std::ofstream file(ignored_list_path, std::ios::binary);
  if(!file.is_open())
    throw std::runtime_error("Could not write to \"" + ignored_list_path.string() + "\".");
  file << json_object;
}

void ReverseDeployer::readManagedFiles()
{
  const sfs::path managed_files_path = source_path_ / managed_files_name_;
  std::ifstream file(managed_files_path, std::ios::binary);
  if(!file.is_open())
    throw std::runtime_error("Could not read \"" + managed_files_path.string() + "\".");
  Json::Value json_object;
  file >> json_object;

  managed_files_.clear();
  deployed_profile_ = json_object["deployed_profile"].asInt();
  separate_profile_dirs_ = json_object["separate_profile_dirs"].asBool();
  for(int prof = 0; prof < json_object["managed_files"].size(); prof++)
  {
    managed_files_.push_back({});
    for(int i = 0; i < json_object["managed_files"][prof]["files"].size(); i++)
    {
      const sfs::path path = json_object["managed_files"][prof]["files"][i]["path"].asString();
      const bool enabled = json_object["managed_files"][prof]["files"][i]["enabled"].asBool();
      managed_files_[prof][path] = enabled;
    }
  }
  updateCurrentLoadorder();
}

void ReverseDeployer::writeManagedFiles() const
{
  Json::Value json_object;
  json_object["separate_profile_dirs"] = separate_profile_dirs_;
  json_object["deployed_profile"] = deployed_profile_;
  for(int prof = 0; prof < managed_files_.size(); prof++)
  {
    json_object["managed_files"][prof]["profile"] = prof;
    for(const auto& [i, pair] : str::enumerate_view(managed_files_[prof]))
    {
      const auto& [path, enabled] = pair;
      json_object["managed_files"][prof]["files"][(int)i]["path"] = path.string();
      json_object["managed_files"][prof]["files"][(int)i]["enabled"] = enabled;
    }
  }

  const sfs::path managed_files_path = source_path_ / managed_files_name_;
  std::ofstream f_stream(managed_files_path, std::ios::binary);
  if(!f_stream.is_open())
    throw std::runtime_error("Could not open \"" + managed_files_path.string() + "\".");
  f_stream << json_object;
}

void ReverseDeployer::updateFilesInDir(const sfs::path& target_dir,
                                       const std::unordered_set<sfs::path>& deployed_files,
                                       sfs::path current_deployer_path,
                                       bool update_ignored_files)
{
  std::vector<sfs::path> dirs;
  std::vector<sfs::path> files;

  std::unordered_set<sfs::path> new_deployed_files;
  bool found_new_deployer = false;
  if(sfs::exists(target_dir / deployed_files_name_))
  {
    current_deployer_path = target_dir;
    found_new_deployer = true;
    // TODO: Progress Notes
    for(const auto& path : std::views::keys(loadDeployedFiles({}, target_dir)))
      new_deployed_files.insert(target_dir / path);
  }
  const std::unordered_set<sfs::path>& current_deployed_files =
    found_new_deployer ? new_deployed_files : deployed_files;

  for(const auto& dir_entry : sfs::directory_iterator(target_dir))
  {
    if(dir_entry.is_directory())
      dirs.push_back(dir_entry.path());
    else
      files.push_back(dir_entry.path());
  }

  for(const auto& file : files)
  {
    const sfs::path file_name = file.filename();
    const sfs::path path_relative_to_target = pu::getRelativePath(file, dest_path_);
    if(file_name == deployed_files_name_ || file_name == ignore_list_file_name_ ||
       file_name.extension() == backup_extension_)
      continue;
    if(ignored_files_.contains(path_relative_to_target) || current_deployed_files.contains(file))
    {
      if(current_profile_ > -1 && current_profile_ < managed_files_.size() &&
         managed_files_[current_profile_].contains(path_relative_to_target))
        managed_files_[current_profile_].erase(path_relative_to_target);
      continue;
    }
    if(update_ignored_files)
      ignored_files_.insert(path_relative_to_target);
    else
    {
      if(!separate_profile_dirs_)
      {
        for(int prof = 0; prof < managed_files_.size(); prof++)
        {
          if(!managed_files_[prof].contains(path_relative_to_target))
            managed_files_[prof][path_relative_to_target] = true;
        }
      }
      else if(!managed_files_[current_profile_].contains(path_relative_to_target))
        managed_files_[current_profile_][path_relative_to_target] = true;
    }
  }

  for(const auto& dir : dirs)
    updateFilesInDir(dir, current_deployed_files, current_deployer_path, update_ignored_files);
}

void ReverseDeployer::moveFilesFromTargetToSource() const
{
  bool move_failed = false;
  for(const auto& [path, enabled] : current_loadorder_)
  {
    const sfs::path full_dest_path = dest_path_ / path;
    const sfs::path full_source_path = getSourcePath(path);
    if(!sfs::exists(full_dest_path))
    {
      log_(Log::LOG_DEBUG,
           std::format("Deployer {} could not find file {}.", name_, full_dest_path.string()));
      continue;
    }
    if(sfs::exists(full_dest_path) && sfs::exists(full_source_path) &&
       (deploy_mode_ == hard_link && sfs::equivalent(full_source_path, full_dest_path) ||
        deploy_mode_ == sym_link && sfs::read_symlink(full_dest_path) == full_source_path))
    {
      continue;
    }
    sfs::create_directories(full_source_path.parent_path());
    if(move_failed)
    {
      sfs::copy(full_dest_path, full_source_path);
      sfs::remove(full_dest_path);
    }
    else
    {
      try
      {
        sfs::rename(full_dest_path, full_source_path);
      }
      catch(...)
      {
        move_failed = true;
        sfs::copy(full_dest_path, full_source_path);
        sfs::remove(full_dest_path);
      }
    }
  }
  if(move_failed)
    log_(Log::LOG_DEBUG,
         std::format("Deployer {} failed to move file from target to source. Using copy fallback.",
                     name_));
}

void ReverseDeployer::updateCurrentLoadorder()
{
  if(current_profile_ < 0 || current_profile_ >= managed_files_.size())
    return;
  current_loadorder_.clear();
  current_loadorder_.reserve(managed_files_[current_profile_].size());
  for(const auto& [path, enabled] : managed_files_[current_profile_])
    current_loadorder_.emplace_back(path, enabled);
  str::sort(current_loadorder_,
            [](auto& pair_l, auto& pair_r)
            {
              const std::string path_l = pair_l.first.string();
              const std::string path_r = pair_r.first.string();
              const int depth_l = str::count(path_l, sfs::path::preferred_separator);
              const int depth_r = str::count(path_r, sfs::path::preferred_separator);
              if(depth_l == depth_r)
                return path_l < path_r;
              // TODO: Should this be inverted?
              return depth_l < depth_r;
            });
}

void ReverseDeployer::deployManagedFiles()
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Deploying managed files...", name_));
  for(const auto& [path, enabled] : current_loadorder_)
  {
    const sfs::path full_dest_path = dest_path_ / path;
    const sfs::path full_source_path = getSourcePath(path);
    if(sfs::exists(full_dest_path))
      sfs::remove(full_dest_path);
    if(!enabled)
      continue;

    if(deploy_mode_ == hard_link)
      sfs::create_hard_link(full_source_path, full_dest_path);
    else if(deploy_mode_ == sym_link)
      sfs::create_symlink(full_source_path, full_dest_path);
    else
      sfs::copy(full_source_path, full_dest_path);
  }
  deployed_profile_ = current_profile_;
}

sfs::path ReverseDeployer::getSourcePath(const sfs::path& path) const
{
  if(separate_profile_dirs_)
    return source_path_ / std::to_string(current_profile_) / path;
  return source_path_ / path;
}
