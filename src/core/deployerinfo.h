/*!
 * \file deployerinfo.h
 * \brief Contains the DeployerInfo struct.
 */

#pragma once

#include <map>
#include <string>
#include <vector>


/*!
 * \brief Stores a \ref Deployer "deployer's" installed mods and load order.
 */
struct DeployerInfo
{
  /*! \brief Names of the mods managed by this deployer, in their load order. */
  std::vector<std::string> mod_names;
  /*! \brief The \ref Deployer "deployer's" load order. */
  std::vector<std::tuple<int, bool>> loadorder;
  /*! \brief Contains groups of mods which conflict with each other. */
  std::vector<std::vector<int>> conflict_groups;
  /*! \brief If true: Deployer manages its own mods and does not rely on ModdedApplication. */
  bool is_autonomous = false;
  /*! \brief For every mod: A vector of manual tags added to that mod. */
  std::vector<std::vector<std::string>> manual_tags;
  /*! \brief For every mod: A vector of auto tags added to that mod. */
  std::vector<std::vector<std::string>> auto_tags;
  /*! \brief Maps tag names to the number of mods for that tag. */
  std::map<std::string, int> mods_per_tag;
};
