/*!
 * \file appinfo.h
 * \brief Contains the AppInfo struct.
 */

#pragma once

#include "deployer.h"
#include "tagcondition.h"
#include "tool.h"
#include <map>
#include <string>
#include <vector>


/*!
 * \brief Stores information about a ModdedApplication.
 */
struct AppInfo
{
  /*! \brief The \ref ModdedApplication "application's" name. */
  std::string name = "";
  /*! \brief The \ref ModdedApplication "application's" staging directory. */
  std::string staging_dir = "";
  /*! \brief Command used to run the \ref ModdedApplication "application". */
  std::string command = "";
  /*! \brief Number of installed mods of the \ref ModdedApplication "application". */
  int num_mods = 0;
  /*!
   * \brief Names of \ref Deployer "deployers" belonging to the
   * \ref ModdedApplication "application".
   */
  std::vector<std::string> deployers{};
  /*!
   * \brief Types of \ref Deployer "deployers" belonging to the
   * \ref ModdedApplication "application".
   */
  std::vector<std::string> deployer_types{};
  /*!
   * \brief Staging directory of \ref Deployer "deployers" belonging to the
   * \ref ModdedApplication "application".
   */
  std::vector<std::string> target_dirs{};
  /*!
   * \brief Number of mods for each \ref Deployer "deployer" belonging to the
   * \ref ModdedApplication "application".
   */
  std::vector<int> deployer_mods{};
  /*! \brief For every deployer: Determines how files will be deployed to the target directory. */
  std::vector<Deployer::DeployMode> deploy_modes{};
  /*!
   * \brief Name and command for each tool belonging to the
   * \ref ModdedApplication "application".
   */
  std::vector<Tool> tools{};
  /*!
   * \brief Maps the names of all manual tags to the number of mods with that tag in the
   * \ref ModdedApplication "application".
   */
  std::map<std::string, int> num_mods_per_manual_tag;
  /*!
   * \brief Maps the names of all auto tags to the number of mods with that tag in the
   * \ref ModdedApplication "application".
   */
  std::map<std::string, int> num_mods_per_auto_tag;
  /*!
   * \brief Maps all auto tag names to a pair of the expression used and a vector of Tagconditions.
   */
  std::map<std::string, std::pair<std::string, std::vector<TagCondition>>> auto_tags;
  /*! \brief Version of the target application. */
  std::string app_version = "";
  /*!
   *  \brief For every deployer: The source directory used. This is equivalent to the staging directory
   *  for non-autonomous deployers.
   */
  std::vector<std::string> deployer_source_dirs{};
  /*!
   * \brief For every deployer: Whether or not it is case invariant.
   */
  std::vector<bool> deployer_is_case_invariant{};
};
