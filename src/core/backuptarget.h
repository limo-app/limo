/*!
 * \file backuptarget.h
 * \brief Header for the BackupTarget struct.
 */

#pragma once

#include <filesystem>
#include <vector>


/*!
 * \brief Stores information about a backup target.
 */
struct BackupTarget
{
  /*! \brief Path to the target file or directory. */
  std::filesystem::path path;
  /*! \brief Display name for this backup target. */
  std::string target_name;
  /*! \brief Contains display names for all backups for this target. */
  std::vector<std::string> backup_names;
  /*! \brief Contains the currently active backup for every profile. */
  std::vector<int> active_members;
  /*! \brief Active member for current profile. */
  int cur_active_member = 0;

  /*!
   * \brief Constructor.
   * \param path Path to the target file or directory.
   * \param target_name Display name for this backup target.
   * \param backup_names Contains display names for all backups for this target.
   * \param active_members Contains the currently active backup for every profile.
   */
  BackupTarget(const std::filesystem::path& path,
               const std::string& target_name,
               const std::vector<std::string>& backup_names,
               const std::vector<int>& active_members);

  /*!
   * \brief Tests every member of this and other for equality.
   * \param other BackupTarget to compare this to.
   * \return True only if every member of this is equal to the respective member in other.
   */
  bool operator==(const BackupTarget& other) const;
};
