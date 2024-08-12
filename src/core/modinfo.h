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
   * \param id The mod's id.
   * \param name The mod's name.
   * \param version The mod's version.
   * \param install_time Timestamp indicating when the mod was installed.
   * \param local_source Source archive for the mod.
   * \param remote_source URL from where the mod was downloaded.
   * \param remote_update_time Timestamp for when the mod was updated at the remote source.
   * \param size Total size of the installed mod on disk.
   * \param suppress_time Timestamp for when the user requested to suppress current update
   * notifications.
   * \param deployer_names Names of all \ref Deployer "deployers" the mod belongs to.
   * \param deployer_ids Ids of all \ref Deployer "deployers" the mod belongs to.
   * \param statuses The mods activation status for every \ref Deployer "deployer" it belongs to.
   * \param group Group this mod belongs to. If == -1: Mod belongs to no group.
   * \param is_active_member If true: Mod is the active member of it's group.
   * \param man_tags The names of all manual tags for this mod.
   */
  ModInfo(int id,
          const std::string& name,
          const std::string& version,
          const std::time_t& install_time,
          const std::filesystem::path& local_source,
          const std::string& remote_source,
          const std::time_t& remote_update_time,
          unsigned long size,
          const std::time_t& suppress_time,
          const std::vector<std::string>& deployer_names,
          const std::vector<int>& deployer_ids,
          const std::vector<bool>& statuses,
          int group,
          bool is_active_member,
          const std::vector<std::string>& man_tags,
          const std::vector<std::string>& au_tags) :
    mod(id,
        name,
        version,
        install_time,
        local_source,
        remote_source,
        remote_update_time,
        size,
        suppress_time),
    deployers(std::move(deployer_names)), deployer_ids(std::move(deployer_ids)),
    deployer_statuses(statuses), group(group), is_active_group_member(is_active_member),
    manual_tags(man_tags), auto_tags(au_tags)
  {}
};
