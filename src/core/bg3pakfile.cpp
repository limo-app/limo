#include "bg3pakfile.h"
#include "lspakextractor.h"
#include "pathutils.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <ranges>

namespace sfs = std::filesystem;
namespace str = std::ranges;
namespace pu = path_utils;


Bg3PakFile::Bg3PakFile(const sfs::path& source_file, const sfs::path& prefix) :
  source_file_(source_file), source_path_prefix_(prefix)
{
  init();
}

Bg3PakFile::Bg3PakFile(const Json::Value& json_value, const sfs::path& prefix) :
  source_path_prefix_(prefix)
{
  source_file_ = json_value["source_file"].asString();
  modified_time_ = json_value["modified_time"].asInt64();
  if(modified_time_ == getTimestamp(source_path_prefix_ / source_file_))
  {
    for(int i = 0; i < json_value["files"].size(); i++)
      file_list_.push_back(json_value["files"][i].asString());
    for(int i = 0; i < json_value["plugins"].size(); i++)
      plugins_.emplace_back(json_value["plugins"][i]["meta_data_xml"].asString());
  }
  else
    init();
}

const std::vector<Bg3Plugin>& Bg3PakFile::getPlugins() const
{
  return plugins_;
}

Json::Value Bg3PakFile::toJson() const
{
  Json::Value json_value;
  json_value["source_file"] = source_file_.string();
  json_value["modified_time"] = modified_time_;
  for(const auto& [i, plugin] : std::views::enumerate(plugins_))
    json_value["plugins"][(int)i]["meta_data_xml"] = plugin.getXmlString();
  for(const auto& [i, path] : std::views::enumerate(file_list_))
    json_value["files"][(int)i] = path.string();
  return json_value;
}

std::filesystem::path Bg3PakFile::getSourceFile() const
{
  return source_file_;
}

bool Bg3PakFile::timestampsMatch()
{
  return modified_time_ == getTimestamp(source_path_prefix_ / source_file_);
}

std::string Bg3PakFile::getPluginName(const std::string& uuid) const
{
  auto iter = str::find_if(plugins_, [&uuid](auto plugin) { return uuid == plugin.getUuid(); });
  if(iter != plugins_.end())
    return iter->getName();
  return "";
}

bool Bg3PakFile::hasPlugin(const std::string& uuid) const
{
  return str::any_of(plugins_, [&uuid](auto plugin) { return uuid == plugin.getUuid(); });
}

bool Bg3PakFile::pluginConflictsWith(const std::string& plugin_uuid,
                                     const Bg3PakFile& other_file,
                                     const std::string& other_plugin_uuid)
{
  auto iter =
    str::find_if(plugins_, [&plugin_uuid](auto plugin) { return plugin_uuid == plugin.getUuid(); });
  if(iter == plugins_.end())
    return false;
  const auto other_plugins = other_file.getPlugins();
  auto other_iter = str::find_if(other_plugins,
                                 [&other_plugin_uuid](auto plugin)
                                 { return other_plugin_uuid == plugin.getUuid(); });
  if(other_iter == other_plugins.end())
    return false;

  const auto dir = iter->getDirectory();
  const auto other_dir = other_iter->getDirectory();
  std::vector<std::string> plugin_files;
  std::string prefix = "Mods/" + dir;
  for(const auto& file : file_list_)
  {
    if(file.string().starts_with(prefix))
      plugin_files.push_back(pu::getRelativePath(file, prefix));
  }
  std::vector<std::string> other_plugin_files;
  prefix = "Mods/" + other_dir;
  for(const auto& file : other_file.file_list_)
  {
    if(file.string().starts_with(prefix))
      other_plugin_files.push_back(pu::getRelativePath(file, prefix));
  }

  for(const auto& file : plugin_files)
  {
    std::cout << file << std::endl;
    if(file != "meta.lsx" && file != "meta.lsf" &&
       str::find(other_plugin_files, file) != other_plugin_files.end())
      return true;
  }
  return false;
}

bool Bg3PakFile::conflictsWith(const Bg3PakFile& other)
{
  for(const auto& file : file_list_)
  {
    if(str::find(other.file_list_, file) != other.file_list_.end())
      return true;
  }
  return false;
}

time_t Bg3PakFile::getTimestamp(const sfs::path& file)
{
  return std::chrono::system_clock::to_time_t(
    std::chrono::clock_cast<std::chrono::system_clock>(sfs::last_write_time(file)));
}

void Bg3PakFile::init()
{
  modified_time_ = getTimestamp(source_path_prefix_ / source_file_);
  LsPakExtractor extractor(source_path_prefix_ / source_file_);
  extractor.init();
  file_list_ = extractor.getFileList();
  auto is_meta_file = [](const auto& pair)
  { return std::filesystem::path(std::get<1>(pair)).filename() == "meta.lsx"; };
  for(const auto& [index, meta_file] :
      file_list_ | std::views::enumerate | std::views::filter(is_meta_file))
  {
    const std::string xml = extractor.extractFile(index);
    if(Bg3Plugin::isValidPlugin(xml))
      plugins_.emplace_back(xml);
  }
}
