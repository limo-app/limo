/*!
 * \file addmodinfo.h
 * \brief Contains the AddModInfo struct.
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>


/*!
 * \brief Stores data needed to install a new mod.
 */
struct AddModInfo
{
  /*! \brief Name of the new mod. */
  std::string name;
  /*! \brief Version of the new mod. */
  std::string version;
  /*! \brief Installer type to be used. */
  std::string installer;
  /*! \brief Path to the mods files. */
  std::string source_path;
  /*! \brief Ids of deployers to which the new mod will be added. */
  std::vector<int> deployers;
  /*! \brief Id of the mod the group of which the new mod will be added to, or -1 for no group. */
  int group;
  /*! \brief Flags for the installer. */
  int installer_flags;
  /*! \brief If > 0: Remove path components with depth < root_level. */
  int root_level;
  /*! \brief Contains pairs of source and destination paths for installation files. */
  std::vector<std::pair<std::filesystem::path, std::filesystem::path>> files;
  /*! \brief If true: The newly installed mod will replace the mod specified in group. */
  bool replace_mod = false;
  /*! \brief Path to the local archive or directory used to install this mod. */
  std::filesystem::path local_source = "";
  /*! \brief URL from where the mod was downloaded. */
  std::string remote_source = "";
};
