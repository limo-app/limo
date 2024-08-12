/*!
 * \file backupmanager.h
 * \brief Header for the BackupManager class.
 */

#pragma once

#include "backuptarget.h"
#include "log.h"
#include <filesystem>
#include <functional>
#include <json/json.h>
#include <vector>


/*!
 * \brief Handles creation of, deletion of and switching between, bachups.
 */
class BackupManager
{
public:
  /*! \brief Empty default constructor. */
  BackupManager() = default;

  /*!
   * \brief Adds a new target file or directory to be managed.
   * \param path Path to the target file or directory.
   * \param name Display name for this target.
   * \param backup_names Display names for initial backups. Must contain at least one.
   */
  void addTarget(const std::filesystem::path& path,
                 const std::string& name,
                 const std::vector<std::string>& backup_names);
  /*!
   * \brief Adds a backup target which was previously managed by a BackupManager.
   * \param path Path to the target file or directory.
   */
  void addTarget(const std::filesystem::path& path);
  /*!
   * \brief Removes the given target by deleting all relevant backups and config files.
   * \param target_id Target to remove.
   */
  void removeTarget(int target_id);
  /*!
   * \brief Adds a new backup for the given target by copying the currently active backup.
   * \param target_id Target for which to create a new backup.
   * \param name Display name for the new backup.
   * \param source Backup from which to copy files to create the new backup. If -1:
   * copy currently active backup.
   */
  void addBackup(int target_id, const std::string& name, int source = -1);
  /*!
   * \brief Deletes the given backup for given target.
   * \param target_id Target from which to delete a backup.
   * \param backup_id Backup to remove.
   * \param update_dirs If true: Repair the target if it is in an invalid state, e.g. if
   * a backup has been manually deleted.
   */
  void removeBackup(int target_id, int backup_id, bool update_dirs = true);
  /*!
   * \brief Changes the currently active backup for the given target.
   * \param target_id Target for which to change the active backup.
   * \param backup_id New active backup.
   */
  void setActiveBackup(int target_id, int backup_id);
  /*!
   * \brief Sets the active profile to the new profile and changes all active backups if
   * needed.
   * \param profile New active profile.
   */
  void setProfile(int profile);
  /*!
   * \brief Adds a new profile.
   * \param source If this refers to an existing backup: Copy the active backups from that
   * profile.
   */
  void addProfile(int source = -1);
  /*!
   * \brief Removes the given profile.
   * \param profile Profile to be removed.
   */
  void removeProfile(int profile);
  /*!
   * \brief Returns a vector containing information about all managed backup targets.
   * \return The vector.
   */
  std::vector<BackupTarget> getTargets() const;
  /*! \brief Deletes all entries in targets_ as well as all profiles. */
  void reset();
  /*! \brief Returns the number of backup targets. */
  int getNumTargets();
  /*!
   * \brief Returns the number of backups for the given target.
   * \param target_id Backup target.
   * \return The number of backups.
   */
  int getNumBackups(int target_id);
  /*!
   * \brief Setter for the name of a backup belonging to the given target.
   * \param target_id Backup target.
   * \param backup_id Backup to be edited.
   * \param name The new name.
   */
  void setBackupName(int target_id, int backup_id, const std::string& name);
  /*!
   * \brief Setter for the name of a backup target.
   * \param target_id Backup target.
   * \param name The new name.
   */
  void setBackupTargetName(int target_id, const std::string& name);
  /*!
   * \brief Deletes all files in the dest backup and replaces them with the files
   * from the source backup.
   * \param target_id Backup target.
   * \param source_backup Backup from which to copy files.
   * \param dest_backup Target for data deletion.
   */
  void overwriteBackup(int target_id, int source_backup, int dest_backup);
  /*!
   * \brief Setter for log callback.
   * \param new_log New log callback
   */
  void setLog(const std::function<void(Log::LogLevel, const std::string&)>& new_log);

private:
  /*! \brief File extension used for backups. */
  static inline const std::string BAK_EXTENSION = ".lmmbakman";
  /*! \brief File extension used for the files used to store a targets state. */
  static inline const std::string JSON_EXTENSION = BAK_EXTENSION + ".json";
  /*! \brief Contains all managed targets. */
  std::vector<BackupTarget> targets_{};
  /*! \brief Number of profiles. */
  int num_profiles_ = 0;
  /*! \brief Currently active profile. */
  int cur_profile_ = -1;
  /*! \brief Callback for logging. */
  std::function<void(Log::LogLevel, const std::string&)> log_ = [](Log::LogLevel a,
                                                                   const std::string& b) {};

  /*!
   * \brief Ensures consistency with the data on disk.
   *
   * This is accomplished by deleting backups for which
   * no file exists and files on disk which should by filename and extension be a
   * backup but have an invalid id. This is done for all files matching the filename
   * and path of any target.
   */
  void updateDirectories();
  /*!
   * \brief Ensures consistency with the data on disk.
   *
   * This is accomplished by deleting backups for which
   * no file exists and renaming files on disk which should by filename and extension be a
   * backup but have an invalid id. This is done for all files matching the filename
   * and path of the given target.
   * \param target_id Target to check.
   */
  void updateDirectories(int target_id);
  /*! \brief Updates internal state by parsing every targets state file. */
  void updateState();
  /*! \brief Updates every targets state file with the internal state. */
  void updateSettings();
  /*!
   * \brief Writes the given json object to disk.
   * \param path Path to write to.
   * \param settings The json object.
   */
  void writeSettings(const std::filesystem::path& path, const Json::Value& settings) const;
  /*!
   * \brief Reads the given file and creates a json object from the files data.
   * \param path File to read.
   * \return The json object created from the file.
   */
  Json::Value readSettings(const std::filesystem::path& path) const;
  /*!
   * \brief Returns the path to the file which contains state data for the given file
   * or directory.
   * \param path File or directory for which to generate the path.
   * \return The path.
   */
  std::filesystem::path getConfigPath(const std::filesystem::path& path) const;
  /*!
   * \brief Returns the path to the given backup for the given file or directory.
   * \param path Path to a backup target.
   * \param backup Backup id for the given target.
   * \return The path.
   */
  std::filesystem::path getBackupPath(const std::filesystem::path& path, int backup) const;
  /*!
   * \brief Returns the path to the given existing backup for the given target.
   * \param path target Target for which to find the path.
   * \param backup Backup id for the given target.
   * \return The path.
   */
  std::filesystem::path getBackupPath(int target, int backup) const;
};
