#include "reversedeployer.h"
#include "pathutils.h"
#include "json/json.h"
#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
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

void ReverseDeployer::updateManagedFiles(bool write, std::optional<ProgressNode*> progress_node)
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Updating managed files...", name_));
  if(progress_node)
    (*progress_node)->setTotalSteps(std::max(number_of_files_in_target_, 0));
  number_of_files_in_target_ = updateFilesInDir(dest_path_, {}, dest_path_, false, progress_node);
  updateCurrentLoadorder();
  moveFilesFromTargetToSource();
  if(write)
    writeManagedFiles();
}

std::map<int, unsigned long> ReverseDeployer::deploy(std::optional<ProgressNode*> progress_node)
{
  const bool other_profile_is_deployed =
    deployed_profile_ != current_profile_ && deployed_profile_ > -1;
  if(progress_node)
  {
    if(other_profile_is_deployed && separate_profile_dirs_)
      (*progress_node)->addChildren({ 1.0f, 1.0f });
    else
      (*progress_node)->addChildren({ 1.0f });
  }

  if(other_profile_is_deployed)
  {
    const int cur_profile = current_profile_;
    current_profile_ = deployed_profile_;
    if(separate_profile_dirs_)
      updateManagedFiles(false, { &(*progress_node)->child(1) });
    unDeploy();
    current_profile_ = cur_profile;
  }
  if(progress_node)
    updateManagedFiles(false, { &(*progress_node)->child(0) });
  else
    updateManagedFiles(false);
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
  if(deployed_profile_ < 0 || deployed_profile_ >= managed_files_.size())
    return;

  for(const auto& [path, _] : managed_files_[deployed_profile_])
  {
    const sfs::path full_dest_path = dest_path_ / path;
    sfs::remove(full_dest_path);
  }
  deployed_profile_ = -1;
  deployed_loadorder_.clear();
}

void ReverseDeployer::swapChild(int from_index, int to_index)
{
  log_(
    Log::LOG_DEBUG,
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
    sfs::remove_all(source_path_ / std::to_string(profile));
    for(int prof = profile + 1; prof < managed_files_.size() - 1; prof++)
    {
      if(pu::exists(source_path_ / std::to_string(prof)))
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
    current_profile_--;
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

int ReverseDeployer::getNumMods()
{
  return current_loadorder_.size();
}

std::shared_ptr<TreeItem<DeployerEntry>> ReverseDeployer::getLoadorder()
{
  std::shared_ptr<TreeItem<DeployerEntry>> loadorder = std::make_shared<TreeItem<DeployerEntry>>(
    std::make_shared<DeployerEntry>(true, "Root"),
    nullptr);
  for(const auto& [i, enabled] : str::enumerate_view(current_loadorder_ | std::views::values))
    loadorder->emplace_back(std::make_shared<DeployerModInfo>(i, "", "", enabled));
  return loadorder;
}

bool ReverseDeployer::addMod(int mod_id, bool enabled, bool update_conflicts)
{
  log_(
    Log::LOG_DEBUG,
    "WARNING: You are trying to add a mod to an autonomous" " deployer. This will have no effect.");
  return false;
}

bool ReverseDeployer::removeMod(int mod_id)
{
  log_(
    Log::LOG_DEBUG,
    "WARNING: You are trying to remove a mod from an autonomous" " deployer. This will have no " "e"
                                                                                                 "f"
                                                                                                 "f"
                                                                                                 "e"
                                                                                                 "c"
                                                                                                 "t"
                                                                                                 ".");
  return false;
}

bool ReverseDeployer::hasMod(int mod_id)
{
  return false;
}

bool ReverseDeployer::swapMod(int old_id, int new_id)
{
  log_(
    Log::LOG_DEBUG,
    "WARNING: You are trying to swap a mod in an autonomous" " deployer. This will have no " "effec"
                                                                                             "t.");
  return false;
}

std::vector<ConflictInfo> ReverseDeployer::getFileConflicts(
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

std::vector<std::pair<sfs::path, int>> ReverseDeployer::getExternallyModifiedFiles(
  std::optional<ProgressNode*> progress_node) const
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Checking for external changes...", name_));

  const bool no_deployed_profile =
    deployed_profile_ < 0 || deployed_profile_ >= managed_files_.size();
  if(progress_node)
  {
    if(no_deployed_profile)
      (*progress_node)->setTotalSteps(1);
    else
      (*progress_node)->setTotalSteps(deployed_loadorder_.size());
  }
  if(no_deployed_profile)
  {
    if(progress_node)
      (*progress_node)->advance();
    log_(Log::LOG_INFO, "No changes found");
    return {};
  }

  std::vector<std::pair<sfs::path, int>> modified_files;
  for(const auto& [i, pair] : str::enumerate_view(deployed_loadorder_))
  {
    const auto& [path, enabled] = pair;
    if(enabled && !sfs::exists(dest_path_ / path) ||
       !sfs::exists(getSourcePath(path, deployed_profile_)))
      modified_files.emplace_back(path, i);
    if(progress_node)
      (*progress_node)->advance();
  }

  if(modified_files.empty())
    log_(Log::LOG_INFO, "No changes found");
  else
    log_(Log::LOG_INFO, std::format("Found {} modified files", modified_files.size()));

  return modified_files;
}

void ReverseDeployer::keepOrRevertFileModifications(const FileChangeChoices& changes_to_keep)
{
  for(const auto& [path, keep] :
      str::zip_view(changes_to_keep.paths, changes_to_keep.changes_to_keep))
  {
    const sfs::path full_source_path(getSourcePath(path, deployed_profile_));
    const sfs::path full_dest_path(dest_path_ / path);

    if(keep)
      deleteFile(path, deployed_profile_);
    else
    {
      const bool source_exists = sfs::exists(full_source_path);
      const bool dest_exists = pu::exists(full_dest_path);
      if(dest_exists && source_exists)
      {
        log_(
          Log::LOG_DEBUG,
          std::format("Deployer '{}': Tried to restore existing file: '{}'", name_, path.string()));
      }
      else if(dest_exists && !source_exists)
      {
        if(deploy_mode_ == hard_link)
          sfs::create_hard_link(full_dest_path, full_source_path);
        else if(deploy_mode_ == sym_link)
        {
          log_(Log::LOG_ERROR,
               std::format("Deployer '{}': File '{}' could not be restored. File does not exist.",
                           name_,
                           path.string()));
          deleteFile(path, deployed_profile_);
        }
        else
          sfs::copy(full_dest_path, full_source_path);
      }
      else if(source_exists && !dest_exists)
      {
        if(deploy_mode_ == hard_link)
          sfs::create_hard_link(full_source_path, full_dest_path);
        else if(deploy_mode_ == sym_link)
          sfs::create_symlink(full_source_path, full_dest_path);
        else
          sfs::copy(full_source_path, full_dest_path);
      }
      else
      {
        log_(Log::LOG_ERROR,
             std::format("Deployer '{}': File '{}' could not be restored. File does not exist.",
                         name_,
                         path.string()));
        deleteFile(path, deployed_profile_);
      }
    }
  }

  if(deployed_profile_ == current_profile_)
    updateCurrentLoadorder();
  writeManagedFiles();
}

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
  for(int prof = 0; prof < managed_files_.size(); prof++)
  {
    const sfs::path source_dir = getSourcePath("", prof);
    for(const auto& dir_entry : sfs::recursive_directory_iterator(source_dir))
    {
      if(dir_entry.is_directory())
        continue;
      const sfs::path relative_path = pu::getRelativePath(dir_entry.path(), source_dir);
      if(!managed_files_[prof].contains(relative_path))
        managed_files_[prof][relative_path] = true;
    }
  }
  updateCurrentLoadorder();
  writeIgnoredFiles();
  writeManagedFiles();
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
    while(pu::exists(temp_path))
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
      if(prof != current_profile_)
        sfs::remove_all(source_path_ / std::to_string(prof));
    }
    int temp_id = 0;
    sfs::path temp_dir = "rev_depl_temp_dir_" + std::to_string(temp_id);
    while(pu::exists(source_path_ / std::to_string(current_profile_) / temp_dir))
      temp_dir = "rev_depl_temp_dir_" + std::to_string(++temp_id);
    sfs::rename(source_path_ / std::to_string(current_profile_), source_path_ / temp_dir);
    for(const auto& dir_entry : sfs::directory_iterator(source_path_ / temp_dir))
    {
      std::string relative_path = pu::getRelativePath(dir_entry.path(), source_path_ / temp_dir);
      sfs::rename(dir_entry.path(), source_path_ / relative_path);
    }
    sfs::remove_all(source_path_ / temp_dir);
  }
  for(int prof = 0; prof < managed_files_.size(); prof++)
  {
    if(prof != current_profile_)
      managed_files_[prof].clear();
  }
  separate_profile_dirs_ = enabled;
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

int ReverseDeployer::getDeployPriority() const
{
  return 2;
}

bool ReverseDeployer::supportsSorting() const
{
  return false;
}

bool ReverseDeployer::supportsReordering() const
{
  return false;
}

bool ReverseDeployer::supportsModConflicts() const
{
  return false;
}

bool ReverseDeployer::supportsFileConflicts() const
{
  return false;
}

bool ReverseDeployer::supportsFileBrowsing() const
{
  return false;
}

void ReverseDeployer::addModToIgnoreList(int mod_id)
{
  if(mod_id < 0 || mod_id >= current_loadorder_.size())
  {
    log_(Log::LOG_DEBUG,
         std::format("Deployer '{}': Could not find mod with id: {}.", name_, mod_id));
    return;
  }

  sfs::path relative_path = current_loadorder_[mod_id].first;
  const sfs::path source_path = getSourcePath(relative_path, current_profile_);
  sfs::remove(source_path);

  current_loadorder_.erase(current_loadorder_.begin() + mod_id);
  for(int prof = 0; prof < managed_files_.size(); prof++)
  {
    if((prof == current_profile_ || !separate_profile_dirs_) &&
       managed_files_[prof].contains(relative_path))
      managed_files_[current_profile_].erase(relative_path);
  }
  ignored_files_.insert(relative_path);
  writeIgnoredFiles();
  writeManagedFiles();
}

std::vector<std::vector<int>> ReverseDeployer::getValidModActions() const
{
  std::vector<std::vector<int>> valid_actions;
  for(int _ = 0; _ < current_loadorder_.size(); _++)
    valid_actions.push_back({});
  return valid_actions;
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
  number_of_files_in_target_ = json_object["number_of_files_in_target"].asInt();
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
  deployed_loadorder_.clear();
  for(int i = 0; i < json_object["deployed_loadorder"].size(); i++)
  {
    deployed_loadorder_.emplace_back(json_object["deployed_loadorder"][i]["path"].asString(),
                                     json_object["deployed_loadorder"][i]["enabled"].asBool());
  }
  updateCurrentLoadorder();
}

void ReverseDeployer::writeManagedFiles() const
{
  Json::Value json_object;
  json_object["separate_profile_dirs"] = separate_profile_dirs_;
  json_object["deployed_profile"] = deployed_profile_;
  json_object["number_of_files_in_target"] = number_of_files_in_target_;
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
  for(const auto& [i, pair] : str::enumerate_view(deployed_loadorder_))
  {
    const auto& [path, enabled] = pair;
    json_object["deployed_loadorder"][(int)i]["path"] = path.string();
    json_object["deployed_loadorder"][(int)i]["enabled"] = enabled;
  }

  const sfs::path managed_files_path = source_path_ / managed_files_name_;
  if(!sfs::exists(managed_files_path.parent_path()))
    sfs::create_directories(managed_files_path.parent_path());
  std::ofstream f_stream(managed_files_path, std::ios::binary);
  if(!f_stream.is_open())
    throw std::runtime_error("Could not open \"" + managed_files_path.string() + "\".");
  f_stream << json_object;
}

int ReverseDeployer::updateFilesInDir(const sfs::path& target_dir,
                                      const std::unordered_set<sfs::path>& deployed_files,
                                      sfs::path current_deployer_path,
                                      bool update_ignored_files,
                                      std::optional<ProgressNode*> progress_node)
{
  std::vector<sfs::path> dirs;
  std::vector<sfs::path> files;

  std::unordered_set<sfs::path> new_deployed_files;
  bool found_new_deployer = false;
  if(sfs::exists(target_dir / deployed_files_name_))
  {
    current_deployer_path = target_dir;
    found_new_deployer = true;
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
    if(progress_node)
      (*progress_node)->advance();

    const sfs::path file_name = file.filename();
    const sfs::path path_relative_to_target = pu::getRelativePath(file, dest_path_);
    if(file_name == deployed_files_name_ || file_name == ignore_list_file_name_ ||
       file_name.extension() == backup_extension_ || file_name == managed_dir_file_name_)
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

  int total_num_files = files.size();
  for(const auto& dir : dirs)
    total_num_files += updateFilesInDir(
      dir, current_deployed_files, current_deployer_path, update_ignored_files, progress_node);
  return total_num_files;
}

void ReverseDeployer::moveFilesFromTargetToSource() const
{
  bool move_failed = false;
  for(const auto& [path, enabled] : current_loadorder_)
  {
    const sfs::path full_dest_path = dest_path_ / path;
    const sfs::path full_source_path = getSourcePath(path, current_profile_);
    const bool dest_exists = pu::exists(full_dest_path);
    const bool source_exists = sfs::exists(full_source_path);
    if(!dest_exists)
    {
      if(!source_exists)
        log_(Log::LOG_DEBUG,
             std::format("Deployer '{}' could not find file {}.", name_, full_dest_path.string()));
      continue;
    }
    if(dest_exists && source_exists &&
       (deploy_mode_ == hard_link && sfs::equivalent(full_source_path, full_dest_path) ||
        deploy_mode_ == sym_link && sfs::is_symlink(full_dest_path) &&
          sfs::read_symlink(full_dest_path) == full_source_path))
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
              // TODO: Sorting alphabetically might be better
              return depth_l < depth_r;
            });
}

void ReverseDeployer::deployManagedFiles()
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Deploying managed files...", name_));
  for(const auto& [path, enabled] : current_loadorder_)
  {
    const sfs::path full_dest_path = dest_path_ / path;
    const sfs::path full_source_path = getSourcePath(path, current_profile_);

    if(!sfs::exists(full_source_path))
    {
      log_(Log::LOG_ERROR,
           std::format("Deployer '{}': Failed to deploy file '{}'. Source does not exist.",
                       name_,
                       path.string()));
      continue;
    }

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
  deployed_loadorder_ = current_loadorder_;
}

sfs::path ReverseDeployer::getSourcePath(const sfs::path& path, int profile) const
{
  if(separate_profile_dirs_)
    return source_path_ / std::to_string(profile) / path;
  return source_path_ / path;
}

void ReverseDeployer::deleteFile(const sfs::path& path, int profile)
{
  sfs::remove(dest_path_ / path);
  sfs::remove(getSourcePath(path, profile));

  for(int cur_prof = 0; cur_prof < managed_files_.size(); cur_prof++)
  {
    if((!separate_profile_dirs_ || cur_prof == profile) && managed_files_[cur_prof].contains(path))
      managed_files_[cur_prof].erase(path);
  }
}
