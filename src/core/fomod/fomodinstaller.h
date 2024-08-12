/*!
 * \file fomodinstaller.h
 * \brief Header for the FomodInstaller class.
 */

#pragma once

#include "installstep.h"
#include <algorithm>
#include <filesystem>
#include <pugixml.hpp>
#include <ranges>
#include <vector>


/*!
 * \brief The fomod namespace contains classes used for parsing a FOMOD xml file and for
 * creating an installer.
 */
namespace fomod
{
/*!
 * \brief Holds data and functions needed to pass a fomod file.
 */
class FomodInstaller
{
public:
  /*! \brief Default constructor. */
  FomodInstaller() = default;

  /*!
   * \brief Initializes the installer.
   * \param config_file Fomod file to be parsed.
   * \param target_path Installation target, this is only used to check file dependencies.
   */
  void init(const std::filesystem::path& config_file,
            const std::filesystem::path& target_path = "",
            const std::string& app_version = "");
  /*!
   * \brief Advances installation process by one step.
   * \param selection For every group: for every plugin: True if selected.
   * \return The next installation step, if one exists.
   */
  std::optional<InstallStep> step(const std::vector<std::vector<bool>>& selection = {});
  /*!
   * \brief Returns a pair of the previous installation step and the
   * selections made at that step.
   * \return The step, if one exists.
   */
  std::optional<std::pair<std::vector<std::vector<bool>>, InstallStep>> stepBack();
  /*!
   * \brief Checks if there is at least one more valid installation step.
   * \param selection Current plugin selection.
   * \return True if more steps exist.
   */
  bool hasNextStep(const std::vector<std::vector<bool>>& selection) const;
  /*!
   * \brief Returns all files to be installed with current selection.
   * \param selection For every group: for every plugin: True if selected.
   * \return Pair or source, destination paths for every file.
   */
  std::vector<std::pair<std::filesystem::path, std::filesystem::path>> getInstallationFiles(
    const std::vector<std::vector<bool>>& selection = {});
  /*!
   * \brief Checks if there is a previous installation step.
   * \return True if there is one.
   */
  bool hasPreviousStep() const;
  /*!
   * \brief Checks if installation has not steps.
   * \return True if no steps where found.
   */
  bool hasNoSteps() const;
  /*!
   * \brief Extracts mod name and version from a fomod info file in path/fomod/info.xml
   * \param path Mod root directory.
   * \return Mod name and version.
   */
  static std::pair<std::string, std::string> getMetaData(const std::filesystem::path& path);

private:
  /*! \brief Source fomod config file. */
  pugi::xml_document config_file_;
  /*! \brief Root node of the config file. */
  pugi::xml_node config_;
  /*! \brief Path used to check for file dependencies. */
  std::filesystem::path target_path_;
  /*! \brief Contains all files extracted from the config file. */
  std::vector<File> files_;
  /*! \brief Steps performed during installation. */
  std::vector<InstallStep> steps_;
  /*! \brief Current installation step. */
  int cur_step_ = -1;
  /*! \brief Maps flags to their value. */
  std::map<std::string, std::string> flags_;
  /*! \brief Base path of the mod to be installed. */
  std::filesystem::path mod_base_path_;
  /*! \brief Previous selections made during installation process. */
  std::vector<std::vector<std::vector<bool>>> prev_selections_;
  /*! \brief Used to evaluate game version conditions. */
  std::function<bool(std::string)> version_eval_fun_ = [](auto s) { return true; };
  /*! \brief Used to evaluate fomm version conditions. */
  std::function<bool(std::string)> fomm_eval_fun_ = [](auto s) { return true; };

  /*!
   * \brief Extracts all files from given file list node and appends them to given vector.
   * \param file_list Source file list.
   * \param target_list Extracted files will be appended to this vector.
   * \param warn_missing If true: Warn if a file is missing.
   */
  void parseFileList(const pugi::xml_node& file_list,
                     std::vector<File>& target_vector,
                     bool warn_missing = true);
  /*!
   * \brief Extracts all install steps from given node and stores them in steps_.
   * \param steps Source node.
   */
  void parseInstallSteps(const pugi::xml_node& steps);
  /*!
   * \brief Determines group type from given string.
   * \param type Source string.
   * \return The type.
   */
  PluginGroup::Type parseGroupType(const std::string& type);
  /*! \brief Updates files_ according to the fomod files conditionalFileInstalls node. */
  void parseInstallList();
  /*!
   * \brief Initializes given plugin plugin from fomod node.
   * \param xml_node Source node.
   * \param plugin Target plugin.
   */
  void initPlugin(const pugi::xml_node& xml_node, Plugin& plugin);
  /*!
   * \brief Determines plugin type from given string.
   * \param type Source string.
   * \return The type.
   */
  PluginType parsePluginType(const std::string& type);
  /*!
   * \brief Updates flags_ and files_ with selection.
   * \param selection For every group: for every plugin: True if selected.
   */
  void updateState(const std::vector<std::vector<bool>>& selection);
  /*!
   * \brief Tries to find fomod/file_name in the given path.
   * \param source Path to check.
   * \param file_name File name to search for.
   * \return Name of fomod directory and file, adapted to the actual capitalization.
   */
  static std::pair<std::string, std::string> getFomodPath(
    const std::filesystem::path& source,
    const std::string& file_name = "ModuleConfig.xml");
  /*!
   * \brief Sorts given vector according to given ordering type.
   * \param source Vector to be sorted.
   * \param order Ordering type.
   */
  template<typename T>
  void sortVector(std::vector<T>& source, std::string order)
  {
    if(order == "Explicit")
      return;
    else if(order == "Descending")
      std::ranges::sort(source, [](auto a, auto b) { return a.name > b.name; });
    else
      std::ranges::sort(source, [](auto a, auto b) { return a.name < b.name; });
  }
};
}
