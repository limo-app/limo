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
  /*! \brief Describes what import action should be taken. */
  enum Type
  {
    download = 0,
    extract = 1
  };
  /*! \brief Target ModdedApplication */
  int app_id;
  /*! \brief Type of action to be performed. */
  Type type;
  /*! \brief Path to the local file used for extraction or empty if type == download. */
  std::filesystem::path local_source;
  /*!
   *  \brief URL used to download the mod. Can be either a URL pointing to the mod itself or
   *  a NexusMods nxm URL.
   */
  std::string remote_source = "";
  /*! \brief This is where the mod should be stored after extraction/ download. */
  std::filesystem::path target_path;
  /*! \brief If remote_source is a NexusMods mod page: The id of the file to be downloaded, else:
   * Not set. */
  int nexus_file_id = -1;
  /*! \brief If !=-1: The mod should be added to this mods group after installation. */
  int mod_id = -1;
  /*! \brief Time at which this object was added to the queue. Used for sorting. */
  std::chrono::time_point<std::chrono::high_resolution_clock> queue_time =
    std::chrono::high_resolution_clock::now();
  /*! \brief If this is not empty: Use this as mod version. */
  std::string version_overwrite = "";

  /*!
   * \brief Compares with another ImportModInfo object by their type.
   * \param other Object to compare to.
   * \return True if only this object has type extract, else false.
   */
  bool operator<(const ImportModInfo& other) const
  {
    if(type == other.type)
      return queue_time > other.queue_time;
    return type < other.type;
  }
};
