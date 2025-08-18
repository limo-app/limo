#include "casematchingdeployer.h"
#include "pathutils.h"
#include <algorithm>
#include <format>

namespace sfs = std::filesystem;
namespace pu = path_utils;


CaseMatchingDeployer::CaseMatchingDeployer(const sfs::path& source_path,
                                           const sfs::path& dest_path,
                                           const std::string& name,
                                           DeployMode deploy_mode) :
  Deployer(source_path, dest_path, name, deploy_mode)
{
  type_ = "Case Matching Deployer";
}

std::map<int, unsigned long> CaseMatchingDeployer::deploy(
  const std::vector<int>& loadorder,
  std::optional<ProgressNode*> progress_node)
{
  if(progress_node)
    (*progress_node)->addChildren({ 2, 1, 3 });
  adaptLoadorderFiles(loadorder,
                      progress_node ? &(*progress_node)->child(0) : std::optional<ProgressNode*>{});
  updateConflictGroups(progress_node ? &(*progress_node)->child(1)
                                     : std::optional<ProgressNode*>{});
  return Deployer::deploy(
    loadorder, progress_node ? &(*progress_node)->child(2) : std::optional<ProgressNode*>{});
}

void CaseMatchingDeployer::updateDeployedFilesForMod(
  int mod_id,
  std::optional<ProgressNode*> progress_node) const
{
  std::map<sfs::path, int> deployed_files = loadDeployedFiles(progress_node);
  for(const auto& [path, id] : deployed_files)
  {
    if(id != mod_id)
      continue;
    const sfs::path dest_path = dest_path_ / path;
    auto actual_path = pu::pathExists(path, source_path_ / std::to_string(mod_id));
    if(!actual_path)
      continue;
    const sfs::path source_path = source_path_ / std::to_string(mod_id) / *actual_path;

    if(sfs::exists(dest_path) && sfs::is_directory(dest_path) || sfs::is_directory(source_path))
      continue;

    sfs::remove(dest_path);

    if(deploy_mode_ == sym_link)
      sfs::create_symlink(source_path, dest_path);
    else if(deploy_mode_ == DeployMode::copy)
      sfs::copy(source_path, dest_path);
    else
      sfs::create_hard_link(source_path, dest_path);
  }
}

bool CaseMatchingDeployer::isCaseInvariant() const
{
  return true;
}

void CaseMatchingDeployer::adaptDirectoryFiles(const sfs::path& path,
                                               int mod_id,
                                               const sfs::path& target_path) const
{
  std::vector<sfs::path> directories;
  for(auto const& dir_entry : sfs::directory_iterator(source_path_ / std::to_string(mod_id) / path))
  {
    const std::string relative_path =
      pu::getRelativePath(dir_entry.path(), source_path_ / std::to_string(mod_id));
    if(pu::exists(target_path / relative_path))
    {
      if(sfs::is_directory(target_path / relative_path))
        directories.push_back(relative_path);
      continue;
    }
    std::string file_name = std::prev(dir_entry.path().end())->string();
    int num_matches = 0;
    std::string match_file_name = file_name;
    if(!pu::exists(target_path / path))
      continue;
    for(const auto& dest_entry : sfs::directory_iterator(target_path / path))
    {
      // This method of determining the file name also works for directory names
      std::string dest_file_name = std::prev(dest_entry.path().end())->string();
      if(!std::equal(file_name.begin(),
                     file_name.end(),
                     dest_file_name.begin(),
                     dest_file_name.end(),
                     [](char a, char b) { return std::tolower(a) == std::tolower(b); }))
        continue;
      num_matches++;
      match_file_name = dest_file_name;
      if(num_matches > 1)
        break;
    }
    if(num_matches == 1)
    {
      const auto source = source_path_ / std::to_string(mod_id) / path / file_name;
      const auto target = source_path_ / std::to_string(mod_id) / path / match_file_name;
      if(!pu::exists(target))
        sfs::rename(source, target);
      else if(sfs::is_directory(target))
        pu::moveFilesToDirectory(source, target);
      else if(match_file_name != file_name)
        throw std::runtime_error(std::format("Could not rename file '{}' to '{}' "
                                             "because the target already exists",
                                             source.string(),
                                             target.string()));
    }
    if(sfs::is_directory(source_path_ / std::to_string(mod_id) / path / match_file_name))
      directories.push_back(path / match_file_name);
  }
  for(const auto& dir : directories)
    adaptDirectoryFiles(dir, mod_id, target_path);
}

void CaseMatchingDeployer::adaptLoadorderFiles(const std::vector<int>& loadorder,
                                               std::optional<ProgressNode*> progress_node) const
{
  log_(Log::LOG_INFO, std::format("Deployer '{}': Matching file names...", name_));
  if(progress_node)
  {
    (*progress_node)->addChildren({ 2, 1 });
    (*progress_node)->child(0).setTotalSteps(loadorder.size());
    (*progress_node)->child(1).setTotalSteps(loadorder.size());
  }
  for(int mod_id : loadorder)
  {
    if(checkModPathExistsAndMaybeLogError(mod_id))
      adaptDirectoryFiles("", mod_id, dest_path_);
    if(progress_node)
      (*progress_node)->child(0).advance();
  }

  std::map<std::string, std::string> file_name_map;
  for(int mod_id : loadorder)
  {
    const sfs::path mod_path = source_path_ / std::to_string(mod_id);
    std::vector<sfs::path> mod_paths;
    for(const auto& dir_entry : sfs::recursive_directory_iterator(mod_path))
      mod_paths.push_back(dir_entry.path());
    std::sort(mod_paths.begin(),
              mod_paths.end(),
              [](const std::string& a, const std::string& b) { return a.size() > b.size(); });
    for(const auto& path : mod_paths)
    {
      const std::string relative_path = pu::getRelativePath(path, mod_path);
      const std::string file_name = std::prev(sfs::path(relative_path).end())->string();
      std::string lower_case_path = pu::toLowerCase(relative_path);
      if(file_name_map.contains(lower_case_path))
      {
        const sfs::path target_file_name =
          std::prev(sfs::path(file_name_map[lower_case_path]).end())->string();
        if(file_name == target_file_name)
          continue;
        const sfs::path source = mod_path / relative_path;
        const sfs::path target =
          mod_path / sfs::path(relative_path).parent_path() / target_file_name;
        if(!pu::exists(target))
          sfs::rename(source, target);
        else if(sfs::is_directory(target))
          pu::moveFilesToDirectory(source, target);
        else
          throw std::runtime_error(std::format("Could not rename file '{}' to '{}' "
                                               "because the target already exists",
                                               source.string(),
                                               target.string()));
      }
      else
        file_name_map[lower_case_path] = relative_path;
    }
    if(progress_node)
      (*progress_node)->child(1).advance();
  }
}

bool CaseMatchingDeployer::supportsExpandableItems() const
{
  return true;
}
