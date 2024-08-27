#include "fomodinstaller.h"
#include "../log.h"
#include "../pathutils.h"
#include <algorithm>
#include <format>
#include <ranges>
#include <regex>

using namespace fomod;
namespace sfs = std::filesystem;
namespace pu = path_utils;


void FomodInstaller::init(const sfs::path& config_file,
                          const sfs::path& target_path,
                          const std::string& app_version)
{
  if(!app_version.empty())
    version_eval_fun_ = [app_version](std::string version) { return app_version == version; };
  cur_step_ = -1;
  config_file_.reset();
  files_.clear();
  steps_.clear();
  int cur_step_ = -1;
  flags_.clear();
  prev_selections_.clear();
  target_path_ = target_path;
  if(sfs::is_directory(config_file))
  {
    mod_base_path_ = config_file;
    auto [fomod_dir_name, config_file_name] = getFomodPath(config_file);
    config_file_.load_file((config_file / fomod_dir_name / config_file_name).c_str());
  }
  else
  {
    mod_base_path_ = config_file.parent_path().parent_path();
    config_file_.load_file(config_file.c_str());
  }
  config_ = config_file_.child("config");
  auto file_list = config_.child("requiredInstallFiles");
  if(file_list)
    parseFileList(file_list, files_);
  auto steps = config_.child("installSteps");
  if(steps)
    parseInstallSteps(steps);
}

std::optional<InstallStep> FomodInstaller::step(const std::vector<std::vector<bool>>& selection)
{
  updateState(selection);
  for(int i = cur_step_ + 1; i < steps_.size(); i++)
  {
    if(steps_[i].dependencies.evaluate(target_path_, flags_, version_eval_fun_, fomm_eval_fun_))
    {
      for(auto& group : steps_[i].groups)
      {
        for(auto& plugin : group.plugins)
          plugin.updateType(target_path_, flags_, version_eval_fun_, fomm_eval_fun_);
      }
      if(cur_step_ > -1)
        prev_selections_.push_back(selection);
      cur_step_ = i;
      return steps_[i];
    }
  }
  return {};
}

std::optional<std::pair<std::vector<std::vector<bool>>, InstallStep>> FomodInstaller::stepBack()
{
  if(cur_step_ < 1)
    return {};
  files_.clear();
  flags_.clear();
  for(auto& step : steps_)
  {
    for(auto& group : step.groups)
    {
      for(auto& plugin : group.plugins)
        plugin.updateType(target_path_, flags_, version_eval_fun_, fomm_eval_fun_);
    }
  }
  if(prev_selections_.size() == 1)
  {
    cur_step_ = -1;
    auto old_selection = prev_selections_[0];
    prev_selections_.clear();
    return { { old_selection, *step() } };
  }
  cur_step_ = -1;
  auto old_selections = prev_selections_;
  prev_selections_.clear();
  step();
  for(int i = 0; i < old_selections.size() - 2; i++)
    step(old_selections[i]);
  int idx = old_selections.size() - 2;
  return { { old_selections[idx + 1], *step(old_selections[idx]) } };
}

bool FomodInstaller::hasNextStep(const std::vector<std::vector<bool>>& selection) const
{
  if(cur_step_ == steps_.size() - 1)
    return false;
  std::map<std::string, std::string> cur_flags = flags_;
  int group_idx = 0;
  if(!selection.empty())
  {
    for(auto& group : steps_[cur_step_].groups)
    {
      int plugin_idx = 0;
      for(auto& plugin : group.plugins)
      {
        if(!selection[group_idx][plugin_idx])
        {
          plugin_idx++;
          continue;
        }
        for(const auto& [key, value] : plugin.flags)
          cur_flags[key] = value;
        plugin_idx++;
      }
      group_idx++;
    }
  }
  for(int i = cur_step_ + 1; i < steps_.size(); i++)
  {
    if(steps_[i].dependencies.evaluate(target_path_, cur_flags, version_eval_fun_, fomm_eval_fun_))
      return true;
  }
  return false;
}

bool FomodInstaller::hasNoSteps() const
{
  return steps_.empty();
}

std::pair<std::string, std::string> FomodInstaller::getMetaData(const sfs::path& path)
{
  pugi::xml_document doc;
  auto [dir_name, file_name] = getFomodPath(path, "info.xml");
  doc.load_file((path / dir_name / file_name).c_str());
  return { doc.child("fomod").child_value("Name"), doc.child("fomod").child_value("Version") };
}

std::vector<std::pair<sfs::path, sfs::path>> FomodInstaller::getInstallationFiles(
  const std::vector<std::vector<bool>>& selection)
{
  updateState(selection);
  parseInstallList();
  std::vector<std::pair<sfs::path, sfs::path>> files;
  for(const auto& file : files_)
    files.emplace_back(file.source, file.destination);
  return files;
}

bool FomodInstaller::hasPreviousStep() const
{
  return cur_step_ > 0;
}

void FomodInstaller::parseFileList(const pugi::xml_node& file_list,
                                   std::vector<File>& target_vector,
                                   bool warn_missing)
{
  for(auto file : file_list.children())
  {
    File new_file;
    const auto source_path = pu::normalizePath(file.attribute("source").value());
    auto source_path_optional = pu::pathExists(source_path, mod_base_path_);
    if(!source_path_optional)
    {
      if(warn_missing)
        Log::warning(std::format("Fomod requires installation of non existent file '{}'",
                                 (mod_base_path_ / source_path).string()));
      continue;
    }
    new_file.source = *source_path_optional;
    auto dest = file.attribute("destination");
    if(dest)
      new_file.destination = pu::normalizePath(dest.value());
    else
      new_file.destination = new_file.source;
    auto always_install = file.attribute("alwaysInstall");
    if(always_install)
      new_file.always_install = always_install.as_bool();
    auto install_if_usable = file.attribute("installIfUsable");
    if(install_if_usable)
      new_file.install_if_usable = install_if_usable.as_bool();
    auto priority = file.attribute("priority");
    if(priority)
      new_file.priority = priority.as_int();
    target_vector.push_back(new_file);
  }
}

void FomodInstaller::parseInstallSteps(const pugi::xml_node& steps)
{
  for(const auto& step : steps.children())
  {
    InstallStep cur_step;
    cur_step.name = step.attribute("name").value();
    if(step.child("visible"))
      cur_step.dependencies = *(step.child("visible").children().begin());
    for(const auto& group : step.child("optionalFileGroups").children())
    {
      PluginGroup cur_group;
      cur_group.name = group.attribute("name").value();
      cur_group.type = parseGroupType(group.attribute("type").value());
      for(const auto& plugin : group.child("plugins").children())
      {
        Plugin cur_plugin;
        initPlugin(plugin, cur_plugin);
        cur_group.plugins.push_back(cur_plugin);
      }
      sortVector(cur_group.plugins, group.child("plugins").attribute("order").value());
      cur_step.groups.push_back(cur_group);
    }
    sortVector(cur_step.groups, step.child("optionalFileGroups").attribute("order").value());
    steps_.push_back(cur_step);
  }
  sortVector(steps_, steps.attribute("order").value());
}

PluginGroup::Type FomodInstaller::parseGroupType(const std::string& type)
{
  if(type == "SelectAtLeastOne")
    return PluginGroup::at_least_one;
  else if(type == "SelectAtMostOne")
    return PluginGroup::at_most_one;
  else if(type == "SelectExactlyOne")
    return PluginGroup::exactly_one;
  else if(type == "SelectAll")
    return PluginGroup::all;
  return PluginGroup::any;
}

void FomodInstaller::parseInstallList()
{
  auto root = config_.child("conditionalFileInstalls");
  if(!root)
    return;
  for(const auto& pattern : root.child("patterns").children())
  {
    if(!Dependency(pattern.child("dependencies"))
          .evaluate(target_path_, flags_, version_eval_fun_, fomm_eval_fun_))
      continue;
    std::vector<File> cur_files;
    parseFileList(pattern.child("files"), cur_files);
    for(const auto& file : cur_files)
    {
      auto duplicate_iter =
        std::ranges::find_if(files_,
                             [file = file](const File& other)
                             {
                               return file.source.string() == other.source.string() &&
                                      file.destination.string() == other.destination.string();
                             });

      if(duplicate_iter == files_.end())
        files_.push_back(file);
    }
  }
  std::stable_sort(files_.begin(), files_.end());
}

void FomodInstaller::initPlugin(const pugi::xml_node& xml_node, Plugin& plugin)
{
  plugin.name = xml_node.attribute("name").value();
  plugin.description = xml_node.child_value("description");
  std::string image_path = xml_node.child("image").attribute("path").value();
  if(image_path.empty())
    plugin.image_path = "";
  else
    plugin.image_path = mod_base_path_ / pu::normalizePath(image_path);
  if(xml_node.child("files"))
    parseFileList(xml_node.child("files"), plugin.files, false);
  for(const auto& flag : xml_node.child("conditionFlags").children())
    plugin.flags[flag.attribute("name").value()] = flag.text().as_string();
  if(xml_node.child("typeDescriptor").child("type"))
  {
    auto type =
      parsePluginType(xml_node.child("typeDescriptor").child("type").attribute("name").value());
    plugin.type = type;
    plugin.default_type = type;
  }
  else
  {
    auto type = parsePluginType(xml_node.child("typeDescriptor")
                                  .child("dependencyType")
                                  .child("defaultType")
                                  .attribute("name")
                                  .value());
    plugin.type = type;
    plugin.default_type = type;
    for(const auto& pattern :
        xml_node.child("typeDescriptor").child("dependencyType").child("patterns").children())
    {
      PluginDependency dependency;
      dependency.type = parsePluginType(pattern.child("type").attribute("name").value());
      dependency.dependencies = pattern.child("dependencies");
      plugin.potential_types.push_back(dependency);
    }
  }
}

PluginType FomodInstaller::parsePluginType(const std::string& type)
{
  if(type == "Required")
    return PluginType::required;
  else if(type == "Optional")
    return PluginType::optional;
  else if(type == "Recommended")
    return PluginType::recommended;
  else if(type == "NotUsable")
    return PluginType::not_usable;
  return PluginType::could_be_usable;
}

void FomodInstaller::updateState(const std::vector<std::vector<bool>>& selection)
{
  if(cur_step_ < 0 || selection.empty())
    return;
  for(int group_idx = 0; auto& group : steps_[cur_step_].groups)
  {
    for(int plugin_idx = 0; auto& plugin : group.plugins)
    {
      if(!selection[group_idx][plugin_idx])
      {
        plugin_idx++;
        continue;
      }
      for(const auto& [key, value] : plugin.flags)
        flags_[key] = value;
      for(const auto& file : plugin.files)
      {
        auto duplicate_iter =
          std::ranges::find_if(files_,
                               [file = file](const File& other)
                               {
                                 return file.source.string() == other.source.string() &&
                                        file.destination.string() == other.destination.string();
                               });
        if(duplicate_iter == files_.end())
          files_.push_back(file);
      }
      plugin_idx++;
    }
    group_idx++;
  }
  std::stable_sort(files_.begin(), files_.end());
}

std::pair<std::string, std::string> FomodInstaller::getFomodPath(const sfs::path& source,
                                                                 const std::string& file_name)
{
  std::string fomod_dir_name = "fomod";
  auto str_equals = [](const std::string& a, const std::string& b)
  {
    return std::equal(a.begin(),
                      a.end(),
                      b.begin(),
                      b.end(),
                      [](char c1, char c2) { return tolower(c1) == tolower(c2); });
  };
  for(const auto& dir_entry : sfs::directory_iterator(source))
  {
    if(!dir_entry.is_directory())
      continue;
    const std::string cur_dir = std::prev(dir_entry.path().end())->string();
    if(str_equals(cur_dir, fomod_dir_name))
    {
      fomod_dir_name = cur_dir;
      break;
    }
  }
  if(!sfs::exists(source / fomod_dir_name))
    return { fomod_dir_name, file_name };
  std::string actual_name = file_name;
  for(const auto& dir_entry : sfs::directory_iterator(source / fomod_dir_name))
  {
    if(dir_entry.is_directory())
      continue;
    const std::string cur_file = dir_entry.path().filename();
    if(str_equals(cur_file, file_name))
    {
      actual_name = cur_file;
      break;
    }
  }
  return { fomod_dir_name, actual_name };
}
