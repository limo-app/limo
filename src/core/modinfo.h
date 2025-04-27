/*!
 * \file modinfo.h
 * \brief Contains the ModInfo struct.
 */

#pragma once

#include "mod.h"
#include <filesystem>
#include <string>


/*!
 * \brief Stores information about a mod as well as the group and
 * \ref Deployer "deployers" it belongs to.
 */
struct ModInfo
{
  /*! \brief Contains information about the mod itself. */
  Mod mod;
  /*! \brief Names of all \ref Deployer "deployers" the mod belongs to. */
  std::vector<std::string> deployers;
  /*! \brief Ids of all \ref Deployer "deployers" the mod belongs to. */
  std::vector<int> deployer_ids;
  /*! \brief The mods activation status for every \ref Deployer "deployer" it belongs to. */
  std::vector<bool> deployer_statuses;
  /*! \brief Group this mod belongs to. If == -1: Mod belongs to no group. */
  int group = -1;
  /*! \brief If true: Mod is the active member of its group. */
  bool is_active_group_member = false;
  /*! \brief Contains the names of all manual tags added to this mod. */
  std::vector<std::string> manual_tags;
  /*! \brief Contains the names of all auto tags added to this mod. */
  std::vector<std::string> auto_tags;

  /*!
   * \brief Constructor. Simply initializes members.
   * \param mod Mod object wrapped by this struct.
   * \param deployer_names Names of all \ref Deployer "deployers" the mod belongs to.
   * \param deployer_ids Ids of all \ref Deployer "deployers" the mod belongs to.
   * \param statuses The mods activation status for every \ref Deployer "deployer" it belongs to.
   * \param group Group this mod belongs to. If == -1: Mod belongs to no group.
   * \param is_active_member If true: Mod is the active member of it's group.
   * \param man_tags The names of all manual tags for this mod.
   */
  ModInfo(const Mod& mod,
          const std::vector<std::string>& deployer_names,
          const std::vector<int>& deployer_ids,
          const std::vector<bool>& statuses,
          int group,
          bool is_active_member,
          const std::vector<std::string>& man_tags,
          const std::vector<std::string>& au_tags) :
    mod(mod),
    deployers(deployer_names), deployer_ids(deployer_ids),
    deployer_statuses(statuses), group(group), is_active_group_member(is_active_member),
    manual_tags(man_tags), auto_tags(au_tags)
  {}
};
