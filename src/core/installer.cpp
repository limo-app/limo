#include "installer.h"
#include "compressionerror.h"
#include "pathutils.h"
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <ranges>
#include <regex>
#define _UNIX
#include <dll.hpp>

namespace sfs = std::filesystem;
namespace pu = path_utils;


void Installer::extract(const sfs::path& source_path,
                        const sfs::path& dest_path,
                        std::optional<ProgressNode*> progress_node)
{
  log(Log::LOG_DEBUG, "Beginning extraction");

  if(sfs::is_directory(source_path))
  {
    sfs::create_directories(dest_path);
    if(source_path.parent_path() == dest_path.parent_path())
      sfs::rename(source_path, dest_path);
    else
      sfs::copy(source_path, dest_path, sfs::copy_options::recursive);
    return;
  }

  try
  {
    extractWithProgress(source_path, dest_path, progress_node);
  }
  catch(CompressionError& error)
  {
    std::string extension = source_path.extension().string();
    std::transform(extension.begin(),
                   extension.end(),
                   extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if(extension == ".rar")
    {
      if(sfs::exists(dest_path))
        sfs::remove_all(dest_path);
      extractRarArchive(source_path, dest_path);
    }
    else
      throw error;
  }
  for(const auto& dir_entry : sfs::recursive_directory_iterator(dest_path))
  {
    auto permissions = sfs::perms::owner_read | sfs::perms::owner_write | sfs::perms::group_read |
                       sfs::perms::group_write | sfs::perms::others_read;
    if(dir_entry.is_directory())
      permissions |= sfs::perms::owner_exec | sfs::perms::group_exec | sfs::perms::others_exec;
    sfs::permissions(dir_entry.path(), permissions);
  }
}

unsigned long Installer::install(const sfs::path& source,
                                 const sfs::path& destination,
                                 int options,
                                 const std::string& type,
                                 int root_level,
                                 const std::vector<std::pair<sfs::path, sfs::path>> fomod_files)
{
  log(Log::LOG_DEBUG, "Beginning mod installation");

  if(type != SIMPLEINSTALLER && type != FOMODINSTALLER)
    throw std::runtime_error("Error: Unknown Installer type \"" + type + "\"!");
  unsigned tmp_id = 0;
  sfs::path tmp_dir;
  do
    tmp_dir = destination.parent_path() / (EXTRACT_TMP_DIR + std::to_string(tmp_id));
  while(sfs::exists(tmp_dir) && tmp_id++ < std::numeric_limits<unsigned>::max());
  if(tmp_id == std::numeric_limits<unsigned>::max())
    throw std::runtime_error("Could not create directory!");
  try
  {
    extract(source, tmp_dir, {});
  }
  catch(CompressionError& error)
  {
    sfs::remove_all(tmp_dir);
    throw error;
  }

  if(type == FOMODINSTALLER)
  {
    if(fomod_files.empty())
    {
      sfs::remove_all(tmp_dir);
      throw std::runtime_error("No files to install.");
    }
    if(root_level > 0)
    {
      auto tmp_move_dir = tmp_dir.string() + "." + MOVE_EXTENSION;
      pu::moveFilesWithDepth(tmp_dir, tmp_move_dir, root_level);
      sfs::rename(tmp_move_dir, tmp_dir);
    }

    for(auto iter = fomod_files.begin(); iter != fomod_files.end(); iter++)
    {
      const auto& [source_file, dest_file] = *iter;
      sfs::create_directories(destination / dest_file.parent_path());
      if(!sfs::exists(tmp_dir / source_file))
      {
        sfs::remove_all(destination);
        sfs::remove_all(tmp_dir);
        throw std::runtime_error("Could not find '" + source_file.string() + "'");
      }
      bool contains_no_duplicates = std::find_if(std::next(iter),
                                                 fomod_files.end(),
                                                 [source_file](auto pair) {
                                                   return pair.first == source_file;
                                                 }) == fomod_files.end();
      if(sfs::is_directory(tmp_dir / source_file))
      {
        contains_no_duplicates &= std::find_if(std::next(iter),
                                               fomod_files.end(),
                                               [source_file](auto pair)
                                               {
                                                 sfs::path parent = pair.first.parent_path();
                                                 bool is_duplicate = false;
                                                 while(parent != "" && parent != "/")
                                                 {
                                                   if(parent == source_file)
                                                   {
                                                     is_duplicate = true;
                                                     break;
                                                   }
                                                   parent = parent.parent_path();
                                                 }
                                                 return is_duplicate;
                                               }) == fomod_files.end();

        if(sfs::exists(destination / dest_file))
          pu::moveFilesToDirectory(
            tmp_dir / source_file, destination / dest_file, contains_no_duplicates);
        else
          pu::copyOrMoveFiles(
            tmp_dir / source_file, destination / dest_file, contains_no_duplicates);
      }
      else
      {
        if(sfs::exists(destination / dest_file) && !sfs::is_directory(destination / dest_file))
          sfs::remove(destination / dest_file);
        if(!dest_file.has_filename())
          pu::copyOrMoveFiles(
            tmp_dir / source_file, destination / source_file.filename(), contains_no_duplicates);
        else
          pu::copyOrMoveFiles(
            tmp_dir / source_file, destination / dest_file, contains_no_duplicates);
      }
    }
    sfs::remove_all(tmp_dir);
  }
  else
  {
    if(options & lower_case)
      pu::renameFiles(tmp_dir, tmp_dir, [](unsigned char c) { return std::tolower(c); });
    else if(options & upper_case)
      pu::renameFiles(tmp_dir, tmp_dir, [](unsigned char c) { return std::toupper(c); });
    if(options & single_directory)
    {
      std::vector<sfs::path> directories;
      for(const auto& dir_entry : sfs::recursive_directory_iterator(tmp_dir))
      {
        if(!dir_entry.is_directory())
          sfs::rename(dir_entry.path(), tmp_dir / dir_entry.path().filename());
        else
          directories.push_back(dir_entry.path());
      }
      for(const auto& dir : directories)
      {
        if(sfs::exists(dir))
          sfs::remove_all(dir);
      }
    }

    sfs::create_directories(destination);
    try
    {
      if(root_level == 0)
        sfs::rename(tmp_dir, destination);
      else
        pu::moveFilesWithDepth(tmp_dir, destination, root_level);
    }
    catch(sfs::filesystem_error& error)
    {
      sfs::remove_all(tmp_dir);
      sfs::remove_all(destination);
      throw error;
    }
    catch(std::runtime_error& error)
    {
      sfs::remove_all(tmp_dir);
      sfs::remove_all(destination);
      throw error;
    }
  }
  unsigned long size = 0;
  for(const auto& dir_entry : sfs::recursive_directory_iterator(destination))
    if(dir_entry.is_regular_file())
      size += dir_entry.file_size();
  return size;
}

void Installer::uninstall(const sfs::path& mod_path, const std::string& type)
{
  sfs::remove_all(mod_path);
}

std::vector<sfs::path> Installer::getArchiveFileNames(const sfs::path& path)
{
  std::vector<sfs::path> file_names;
  if(sfs::is_directory(path))
  {
    for(const auto& dir_entry : sfs::recursive_directory_iterator(path))
      file_names.push_back(pu::getRelativePath(dir_entry.path(), path));
    return file_names;
  }
  struct archive* source;
  struct archive_entry* entry;
  source = archive_read_new();
  archive_read_support_filter_all(source);
  archive_read_support_format_all(source);
  if(archive_read_open_filename(source, path.string().c_str(), 10240) != ARCHIVE_OK)
    throw CompressionError("Could not open archive file.");
  while(archive_read_next_header(source, &entry) == ARCHIVE_OK)
    file_names.push_back(archive_entry_pathname(entry));
  if(archive_read_free(source) != ARCHIVE_OK)
    throw CompressionError("Parsing of archive failed.");
  return file_names;
}

std::tuple<int, std::string, std::string> Installer::detectInstallerSignature(
  const sfs::path& source)
{
  const auto path = (sfs::path("fomod") / "ModuleConfig.xml");
  auto str_equals = [](const std::string& a, const std::string& b)
  {
    return std::equal(a.begin(),
                      a.end(),
                      b.begin(),
                      b.end(),
                      [](char c1, char c2) { return tolower(c1) == tolower(c2); });
  };
  const auto files = getArchiveFileNames(source);
  int max_length = 0;
  for(const auto& file : files)
    max_length = std::max(max_length, pu::getPathLength(file));
  for(int root_level = 0; root_level < max_length; root_level++)
  {
    for(const auto& file : files)
    {
      const auto [head, tail] = pu::removePathComponents(file, root_level);
      if(str_equals(path, tail))
        return { root_level, head.string(), FOMODINSTALLER };
    }
  }
  return { 0, {}, SIMPLEINSTALLER };
}

void Installer::cleanupFailedInstallation(const sfs::path& staging_dir, int mod_id)
{
  if(mod_id >= 0)
  {
    if(sfs::exists(staging_dir / std::to_string(mod_id)))
      sfs::remove_all(staging_dir / std::to_string(mod_id));
  }
  for(const auto& dir_entry : sfs::directory_iterator(staging_dir))
  {
    if(!dir_entry.is_directory())
      continue;
    if(dir_entry.path().extension() == MOVE_EXTENSION)
      sfs::remove_all(dir_entry.path());
    std::regex tmp_dir_regex(EXTRACT_TMP_DIR + R"(\d+)");
    if(std::regex_search(dir_entry.path().filename().string(), tmp_dir_regex))
      sfs::remove_all(dir_entry.path());
  }
}

void Installer::setIsAFlatpak(bool is_a_flatpak)
{
  is_a_flatpak_ = is_a_flatpak;
}

void Installer::throwCompressionError(struct archive* source)
{
  throw CompressionError("Error during archive extraction.");

  // The following code sometimes crashes during execution of archive_error_string:

  // throw CompressionError(
  // ("Error during archive extraction: " + std::string(archive_error_string(source))).c_str());
}

void Installer::copyArchive(struct archive* source, struct archive* dest)
{
  int return_code;
  const void* buffer;
  size_t size;
  la_int64_t offset;

  while(true)
  {
    return_code = archive_read_data_block(source, &buffer, &size, &offset);
    if(return_code == ARCHIVE_EOF)
      return;
    if(return_code < ARCHIVE_OK)
      throwCompressionError(source);
    if(archive_write_data_block(dest, buffer, size, offset) < ARCHIVE_OK)
      throwCompressionError(dest);
  }
}

void Installer::extractWithProgress(const sfs::path& source_path,
                                    const sfs::path& dest_path,
                                    std::optional<ProgressNode*> progress_node)
{
  log(Log::LOG_DEBUG, "Beginning extraction with progress");

  constexpr int buffer_size = 10240;
  struct archive* source;
  struct archive* dest;
  struct archive_entry* entry;
  int return_code;
  const char* file_name = source_path.c_str();
  int flags = ARCHIVE_EXTRACT_TIME;
  sfs::path working_dir = "/tmp";
  try
  {
    working_dir = sfs::current_path();
  }
  catch(std::filesystem::filesystem_error& error)
  {}
  if(!sfs::exists(dest_path))
    sfs::create_directories(dest_path);
  sfs::current_path(dest_path);
  source = archive_read_new();
  archive_read_support_format_all(source);
  archive_read_support_filter_all(source);
  dest = archive_write_disk_new();
  archive_write_disk_set_options(dest, flags);
  archive_write_disk_set_standard_lookup(dest);
  if(archive_read_open_filename(source, file_name, buffer_size))
  {
    sfs::current_path(working_dir);
    throw CompressionError("Could not open archive file.");
  }
  uint64_t total_size = 0;
  while(true)
  {
    return_code = archive_read_next_header(source, &entry);
    if(return_code == ARCHIVE_EOF)
      break;
    if(return_code < ARCHIVE_OK)
    {
      sfs::current_path(working_dir);
      throwCompressionError(source);
    }
    total_size += archive_entry_size(entry);
  }
  if(progress_node)
    (*progress_node)->setTotalSteps(total_size);
  archive_read_close(source);
  archive_read_free(source);
  source = archive_read_new();
  archive_read_support_format_all(source);
  archive_read_support_filter_all(source);
  if(archive_read_open_filename(source, file_name, buffer_size))
  {
    sfs::current_path(working_dir);
    throw CompressionError("Could not open archive file.");
  }

  while(true)
  {
    return_code = archive_read_next_header(source, &entry);
    if(return_code == ARCHIVE_EOF)
      break;
    if(return_code < ARCHIVE_OK)
    {
      sfs::current_path(working_dir);
      throwCompressionError(source);
    }
    archive_entry_set_pathname(entry, archive_entry_pathname(entry));
    if(archive_write_header(dest, entry) < ARCHIVE_OK)
    {
      sfs::current_path(working_dir);
      throwCompressionError(dest);
    }
    const void* buff;
    size_t size;
    int64_t offset;

    while(true)
    {
      return_code = archive_read_data_block(source, &buff, &size, &offset);
      if(return_code == ARCHIVE_EOF)
        break;
      if(return_code < ARCHIVE_OK)
      {
        sfs::current_path(working_dir);
        throwCompressionError(source);
      }
      if(archive_write_data_block(dest, buff, size, offset) != ARCHIVE_OK)
      {
        sfs::current_path(working_dir);
        throwCompressionError(dest);
      }
      if(progress_node)
        (*progress_node)->advance(size);
    }
    if(archive_write_finish_entry(dest) < ARCHIVE_OK)
    {
      sfs::current_path(working_dir);
      throwCompressionError(dest);
    }
  }
  archive_read_close(source);
  archive_read_free(source);
  archive_write_close(dest);
  archive_write_free(dest);
  sfs::current_path(working_dir);
}

void Installer::extractRarArchive(const sfs::path& source_path, const sfs::path& dest_path)
{
  log(Log::LOG_DEBUG, "Using fallback rar extraction");

  const auto source_str = source_path.string();
  const auto dest_str = dest_path.string();
  char input_path[source_str.size() + 1];
  for(int i = 0; i < source_str.size(); i++)
    input_path[i] = source_str[i];
  input_path[source_str.size()] = '\0';
  char output_path[dest_str.size() + 1];
  for(int i = 0; i < dest_str.size(); i++)
    output_path[i] = dest_str[i];
  output_path[dest_str.size()] = '\0';

  RAROpenArchiveDataEx archive{ input_path, nullptr, RAR_OM_EXTRACT, 0, nullptr, 0, 0, 0, 0 };
  HANDLE hArcData = RAROpenArchiveEx(&archive);
  if(archive.OpenResult != 0)
    throw CompressionError("Failed to open RAR archive.");
  auto header_data = std::make_unique<RARHeaderDataEx>();
  int header_state = RARReadHeaderEx(hArcData, header_data.get());
  while(header_state == 0)
  {
    if(RARProcessFile(hArcData, RAR_EXTRACT, output_path, nullptr) != 0)
      throw CompressionError("Failed to extract RAR archive.");
    header_state = RARReadHeaderEx(hArcData, header_data.get());
  }
  if(header_state != ERAR_END_ARCHIVE)
    throw CompressionError("Failed to extract RAR archive.");
  RARCloseArchive(hArcData);
}
