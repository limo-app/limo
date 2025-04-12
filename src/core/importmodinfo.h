/*!
 * \file importmodinfo.h
 * \brief Contains the ImportModInfo struct.
 */

#pragma once

#include <chrono>
#include <filesystem>
#include <string>


/*!
 * \brief Stores data needed to download or extract a mod.
 */
struct ImportModInfo
{
  /*! \brief Describes which remote source this mod was retreived from. */
  enum RemoteType
  {
    /*! \brief No remote. */
    local = 0,
    /*! \brief NexusMods. */
    nexus = 1
  };

  /*! \brief Describes what import action should be taken. */
  enum ActionType
  {
    /*! \brief Mod is to be downloaded. */
    download = 0,
    /*! \brief Mod archive is to be extracted. */
    extract = 1,
    /*! \brief Installation dialog is to be shown. */
    install_dialog = 2,
    /*! \brief Mod is to be installed. */
    install = 3
  };

  /*! \brief Target ModdedApplication */
  int app_id;
  /*! \brief ActionType of action to be performed. */
  ActionType action_type;
  /*! \brief Path to the local file used for extraction or empty if type == download. */
  std::filesystem::path local_source;
  /*! \brief Type of remote this mod was retreived from. */
  RemoteType remote_type = local;
  /*! \brief Remote URL associated with this mod. */
  std::string remote_source = "";
  /*! \brief Remote URL used to request a download URL. */
  std::string remote_request_url = "";
  /*! \brief If this was retreived from a remote source: The mod id on the remote. */
  long remote_mod_id = -1;
  /*! \brief If this was retreived from a remote source: The file id on the remote. */
  long remote_file_id = -1;
  /*! \brief If this was retreived from a remote source: The mod name on the remote. */
  std::string remote_mod_name = "";
  /*! \brief If this was retreived from a remote source: The file name on the remote. */
  std::string remote_file_name = "";
  /*! \brief If this was retreived from a remote source: The file version on the remote. */
  std::string remote_file_version = "";
  /*! \brief URL used to download the mod. Note: This may only be valid for a limited time period. */
  std::string remote_download_url = "";
  /*! \brief If !=-1: The mod should be added to this mods group after installation. */
  int target_group_id = -1;
  /*! \brief Id assigned to this mod by Limo. */
  int limo_mod_id = -1;
  /*! \brief This is where the mod should be stored after extraction/ download. */
  std::filesystem::path target_path;
  /*! \brief Current location of the mod on disk. */
  std::filesystem::path current_path;
  /*! \brief Time at which this object was added to the queue. Used for sorting. */
  std::chrono::time_point<std::chrono::high_resolution_clock> queue_time =
    std::chrono::high_resolution_clock::now();
  /*! \brief If this is not empty: Use this as mod version. */
  std::string version_overwrite = "";
  /*! \brief If this is not empty: Use this as mod name. */
  std::string name_overwrite = "";
  /*! \brief Indicates whether the last import action performed was successful. */
  bool last_action_was_successful = true;
  /*! \brief Flags for the installer. */
  int installer_flags = 0;
  /*! \brief If > 0: Remove path components with depth < root_level. */
  int root_level = 0;
  /*! \brief Contains pairs of source and destination paths for installation files. */
  std::vector<std::pair<std::filesystem::path, std::filesystem::path>> files{};
  /*! \brief If true: The newly installed mod will replace the mod specified in group. */
  bool replace_mod = false;
  /*! \brief Ids of deployers to which the new mod will be added. */
  std::vector<int> deployers{};
  /*! \brief Installer type to be used. */
  std::string installer;
  /*! \brief Name of the new mod. */
  std::string name;
  /*! \brief Version of the new mod. */
  std::string version;

  /*!
   * \brief Compares with another ImportModInfo object by their action_type.
   * \param other Object to compare to.
   * \return True if only this object has action_type extract, else false.
   */
  bool operator<(const ImportModInfo& other) const
  {
    if(action_type == other.action_type)
      return queue_time > other.queue_time;
    return action_type < other.action_type;
  }
};
