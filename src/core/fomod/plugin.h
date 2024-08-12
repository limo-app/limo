/*!
 * \file plugin.h
 * \brief Header for the Plugin struct.
 */

#pragma once

#include "file.h"
#include "plugindependency.h"
#include "plugintype.h"
#include <filesystem>
#include <map>
#include <vector>


/*!
 * \brief The fomod namespace contains classes used for parsing a FOMOD xml file and for
 * creating an installer.
 */
namespace fomod
{
/*! \brief Represents one selectable option during installation. */
struct Plugin
{
  /*! \brief Plugin name. */
  std::string name;
  /*! \brief Plugin description. */
  std::string description;
  /*! \brief Path to an image representing this plugin. */
  std::filesystem::path image_path;
  /*! \brief Affects how this plugin is displayed. */
  PluginType type;
  /*! \brief Fallback type if this has potential types but none are valid. */
  PluginType default_type;
  /*! \brief Plugin takes the first type for which the condition is fulfilled. */
  std::vector<PluginDependency> potential_types;
  /*! \brief Flags to be set when this is selected. */
  std::map<std::string, std::string> flags;
  /*! \brief Files to be installed when this is selected. */
  std::vector<File> files;

  /*!
   * \brief Updates type according to potential_types
   * \param target_path Path file conditions.
   * \param current_flags Flags to check.
   * \param version_eval_fun Used to evaluate game version conditions.
   * \param fomm_eval_fun Used to evaluate game fromm conditions.
   */
  void updateType(
    const std::filesystem::path& target_path,
    const std::map<std::string, std::string>& current_flags,
    std::function<bool(std::string)> version_eval_fun,
    std::function<bool(std::string)> fomm_eval_fun = [](auto s) { return true; })
  {
    for(const auto& cur_type : potential_types)
    {
      if(cur_type.dependencies.evaluate(
           target_path, current_flags, version_eval_fun, fomm_eval_fun))
      {
        type = cur_type.type;
        return;
      }
    }
    type = default_type;
  }
};
}
