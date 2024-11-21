#include "pathutils.h"
#include <algorithm>
#include <regex>
#include <set>

namespace sfs = std::filesystem;


namespace path_utils
{
std::optional<sfs::path> pathExists(const sfs::path& path_to_check,
                                    const sfs::path& base_path,
                                    bool case_insensitive)
{
  if(sfs::exists(base_path / path_to_check))
    return path_to_check;
  if(!case_insensitive || base_path != "" && !sfs::exists(base_path))
    return {};
  const sfs::path target =
    path_to_check.string().ends_with("/") ? path_to_check.parent_path() : path_to_check;

  sfs::path actual_path;
  for(auto iter = target.begin(); iter != target.end(); iter++)
  {
    if(sfs::exists(base_path / actual_path / *iter))
    {
      actual_path /= *iter;
      continue;
    }

    std::string lower_part = toLowerCase(*iter);
    bool found = false;
    for(const auto& dir_entry : sfs::directory_iterator(base_path / actual_path))
    {
      const sfs::path path_end = *(std::prev(dir_entry.path().end()));
      std::string lower_case_path_end = toLowerCase(path_end);
      std::string actual_case_path_end = path_end.string();
      if(lower_case_path_end == lower_part)
      {
        actual_path /= actual_case_path_end;
        found = true;
        break;
      }
    }
    if(!found)
      return {};
  }
  return actual_path;
}

std::string toLowerCase(const sfs::path& path)
{
  auto path_string = path.string();
  std::transform(path_string.begin(),
                 path_string.end(),
                 path_string.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return path_string;
}

void moveFilesToDirectory(const sfs::path& source, const sfs::path& destination, bool move)
{
  if(!sfs::exists(destination))
    sfs::create_directories(destination);
  for(const auto& dir_entry : sfs::directory_iterator(source))
  {
    const auto relative_path = getRelativePath(dir_entry.path(), source);
    if(sfs::exists(destination / relative_path))
    {
      if(sfs::is_directory(destination / relative_path))
        moveFilesToDirectory(dir_entry.path(), destination / relative_path, move);
      else
      {
        sfs::remove(destination / relative_path);
        copyOrMoveFiles(dir_entry.path(), destination / relative_path, move);
      }
      continue;
    }
    copyOrMoveFiles(dir_entry.path(), destination / relative_path, move);
  }
  if(sfs::exists(source) && move)
    sfs::remove_all(source);
}

std::string normalizePath(const std::string& path)
{
  return std::regex_replace(path, std::regex(R"(\\)"), "/");
}

std::string getRelativePath(sfs::path target, sfs::path source)
{
  if(source == target)
    return "";
  std::string relative_path = target.string();
  const std::string source_string = source.string();
  const bool ends_with_separator =
    source_string.ends_with(sfs::path::preferred_separator) || source_string.ends_with('/');
  relative_path.erase(0, source_string.size() + (ends_with_separator ? 0 : 1));
  return relative_path;
}

bool directoryIsEmpty(const sfs::path& directory, std::vector<std::string> ignored_files)
{
  if(!sfs::is_directory(directory))
    return false;
  for(const auto& dir_entry : sfs::recursive_directory_iterator(directory))
  {
    if(!dir_entry.is_directory() &&
       !std::ranges::any_of(
         ignored_files, [&dir_entry](auto file) { return file == dir_entry.path().filename(); }))
      return false;
  }
  return true;
}

int getPathLength(const sfs::path& path)
{
  int length = 0;
  for(const auto& e : path)
    length++;
  return length;
}

std::pair<sfs::path, sfs::path> removePathComponents(const sfs::path& path, int depth)
{
  sfs::path short_path;
  sfs::path head;
  int cur_depth = 0;
  for(auto it = path.begin(); it != path.end(); it++, cur_depth++)
  {
    if(cur_depth >= depth)
      short_path /= *it;
    else
      head /= *it;
  }
  return { head, short_path };
}

void renameFiles(const sfs::path& destination,
                 const sfs::path& source,
                 std::function<unsigned char(unsigned char)> converter)
{
  std::vector<sfs::path> old_directories;
  for(const auto& dir_entry : sfs::recursive_directory_iterator(source))
  {
    auto relative_path = getRelativePath(dir_entry.path(), source);
    std::string old_path = relative_path;
    std::transform(relative_path.begin(), relative_path.end(), relative_path.begin(), converter);
    if(dir_entry.is_directory())
    {
      if(old_path != relative_path)
        old_directories.push_back(dir_entry.path());
      continue;
    }
    if(!sfs::exists((destination / relative_path).parent_path()))
      sfs::create_directories((destination / relative_path).parent_path());
    sfs::rename(dir_entry.path(), destination / relative_path);
  }
  if(source == destination)
  {
    for(const auto& dir : old_directories)
    {
      if(sfs::exists(dir))
        sfs::remove_all(dir);
    }
  }
  else
    sfs::remove_all(source);
}

void moveFilesWithDepth(const sfs::path& source, const sfs::path& destination, int depth)
{
  std::set<std::pair<sfs::path, sfs::path>> files_to_move;
  for(const auto& dir_entry : sfs::recursive_directory_iterator(source))
  {
    const auto [head, short_path] =
      removePathComponents(getRelativePath(dir_entry.path(), source), depth);
    if(short_path != "")
      files_to_move.emplace(dir_entry.path(), destination / short_path);
  }

  for(const auto& [cur_source, cur_dest] : files_to_move)
  {
    if(sfs::is_directory(cur_source))
      sfs::create_directories(cur_dest);
    else
    {
      if(sfs::exists(cur_dest))
        throw std::runtime_error("Error: Duplicate file detected: \"" +
                                 getRelativePath(cur_source, source) + "\"!");
      if(cur_dest.has_parent_path())
        sfs::create_directories(cur_dest.parent_path());
      sfs::rename(cur_source, cur_dest);
    }
  }
  sfs::remove_all(source);
}

void copyOrMoveFiles(const sfs::path& source, const sfs::path& destination, bool move)
{
  if(move)
    sfs::rename(source, destination);
  else
    sfs::copy(source, destination, sfs::copy_options(sfs::copy_options::recursive));
}

}
