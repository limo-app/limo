/*!
 * \file deployerinfo.h
 * \brief Contains the DeployerInfo struct.
 */

#pragma once

#include <map>
#include <string>
#include <vector>
#include "treeitem.h"
#include "deployerentry.hpp"

/*!
 * \brief Stores a \ref Deployer "deployer's" installed mods and load order.
 */
struct DeployerInfo
{
  /*! \brief Contains groups of mods which conflict with each other. */
  std::vector<std::vector<int>> conflict_groups;
  /*! \brief If true: Deployer manages its own mods and does not rely on ModdedApplication. */
  bool is_autonomous = false;
  /*! \brief Maps tag names to the number of mods for that tag. */
  std::map<std::string, int> mods_per_tag;

  /*! \brief Root of mod tree */
  std::shared_ptr<TreeItem<DeployerEntry>> root = nullptr;

  /*!
   * \brief Used by ReverseDeployers: If true: Store files on a per profile basis.
   * Else: All profiles use the same files.
   */
  bool separate_profile_dirs = false;
  /*! \brief Used by ReverseDeployers: If true: Deployer has files on the ignore list. */
  bool has_ignored_files = false;
  /*! \brief Whether or not this deployer type supports sorting mods. */
  bool supports_sorting = true;
  /*! \brief Whether or not this deployer type supports reordering mods. */
  bool supports_reordering = true;
  /*! \brief Whether or not this deployer type supports showing mod conflicts. */
  bool supports_mod_conflicts = true;
  /*! \brief Whether or not this deployer type supports showing file conflicts. */
  bool supports_file_conflicts = true;
  /*! \brief Whether or not this deployer type supports browsing mod files. */
  bool supports_file_browsing = true;
  /*! \brief Whether or not this deployer type supports expandable items. */
  bool supports_expandable = true;
  /*! \brief The type of this deployer. */
  std::string type = "";
  /*! \brief Whether or not this deployer type uses mod ids as references to source mods. */
  bool ids_are_source_references = false;
  /*! \brief If ids_are_source_references: For every mod: The source mod's name. Else: Empty */
  std::vector<std::string> source_mod_names_ = {};
  /*! \brief Contains names and icon names for additional actions which can be applied to a mod. */
  std::vector<std::pair<std::string, std::string>> mod_actions = {};
  /*! \brief For every mod: IDs of every valid mod_action which is valid for that mod. */
  std::vector<std::vector<int>> valid_mod_actions = {};
  /*! \brief Determines whether sorting mods can affect overwrite behavior. */
  bool uses_unsafe_sorting = false;
};
