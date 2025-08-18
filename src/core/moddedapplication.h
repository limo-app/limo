/*!
 * \file moddedapplication.h
 * \brief Header for the ModdedApplication class.
 */

#pragma once

#include "appinfo.h"
#include "autotag.h"
#include "backupmanager.h"
#include "deployer.h"
#include "deployerinfo.h"
#include "editautotagaction.h"
#include "editdeployerinfo.h"
#include "editmanualtagaction.h"
#include "editprofileinfo.h"
#include "externalchangesinfo.h"
#include "log.h"
#include "manualtag.h"
#include "modinfo.h"
#include "nexus/api.h"
#include "tool.h"
#include <filesystem>
#include <json/json.h>
#include <string>
#include <vector>


/*!
 * \brief Contains all mods and Deployer objects used for one target application.
 * Stores internal state in a JSON file.
 */
class ModdedApplication
{
public:
  /*!
   * \brief If a JSON settings file already exists in app_mod_dir, it is
   * used to construct this object.
   * \param staging_dir Path to staging directory where all installed mods are stored.
   * \param name Name of target application.
   * \param command Command used to run target application.
   * \param icon_path Path to an icon for this application.
   * \throws Json::LogicError Indicates a logic error, e.g. trying to convert "123" to a bool,
   * while parsing.
   * \throws Json::RuntimeError Indicates a syntax error in the JSON file.
   * \throws ParseError Indicates a semantic error while parsing the JSON file, e.g.
   * the active member of a group is not part of that group.
   */
  ModdedApplication(std::filesystem::path staging_dir,
                    std::string name = "",
                    std::string command = "",
                    std::filesystem::path icon_path = "",
                    std::string app_version = "");

  /*! \brief Name of the file used to store this objects internal state. */
  inline static const std::string CONFIG_FILE_NAME = "lmm_mods.json";

  /*! \brief Deploys mods using all Deployer objects of this application. */
  void deployMods();
  /*!
   * \brief Deploys mods using Deployer objects with given ids.
   * \param deployers The Deployer ids used for deployment.
   */
  void deployModsFor(std::vector<int> deployers);
  /*! \brief Undeploys mods for all managed deployers. */
  void unDeployMods();
  /*!
   * \brief Undeploys mods for the given deployers.
   * \param deployers Target deployers.
   */
  void unDeployModsFor(std::vector<int> deployers);
  /*!
   * \brief Installs a new mod using the given Installer type.
   * \param info Contains all data needed to install the mod.
   */
  void installMod(const ImportModInfo& info);
  /*!
   * \brief Uninstalls the given mods, this includes deleting all installed files.
   * \param mod_id Ids of the mods to be uninstalled.
   * \param installer_type The Installer type used. If an empty string is given, the Installer
   * used during installation is used.
   */
  void uninstallMods(const std::vector<int>& mod_ids, const std::string& installer_type = "");
  void commitChanges();
  /*!
   * \brief Appends a new mod to the load order for given Deployer.
   * \param deployer The target Deployer
   * \param mod_id Id of the mod to be added.
   * \param update_conflicts Updates the target deployers conflict groups only if this is true.
   * \param progress_node Used to inform about the current progress.
   */
  void addModToDeployer(int deployer,
                        int mod_id,
                        bool update_conflicts = true,
                        std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Removes a mod from the load order for given Deployer.
   * \param deployer The target Deployer
   * \param mod_id Id of the mod to be removed.
   * \param update_conflicts Updates the target deployers conflict groups only if this is true.
   * \param progress_node Used to inform about the current progress.
   */
  void removeNodeFromDeployer(int deployer,
                             void *node_ptr,
                             bool update_conflicts = true,
                             std::optional<ProgressNode*> progress_node = {});
  void removeModFromDeployer(int deployer,
                             int mod_id,
                             bool update_conflicts = true,
                             std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Enables or disables the given mod in the load order for given Deployer.
   * \param deployer The target Deployer
   * \param mod_id Mod to be edited.
   * \param status The new status.
   */
  void setModStatus(int deployer, int mod_id, bool status);
  /*!
   * \brief Adds a new Deployer of given type.
   * \param info Contains all data needed to create a deployer, e.g. its name.
   */
  void addDeployer(const EditDeployerInfo& info);
  /*!
   * \brief Removes a Deployer.
   * \param deployer The Deployer.
   * \param cleanup If true: Remove all currently deployed files and restore backups.
   */
  void removeDeployer(int deployer, bool cleanup);
  /*!
   * \brief Creates a vector containing the names of all Deployer objects.
   * \return The vector.
   */
  std::vector<std::string> getDeployerNames() const;
  /*!
   * \brief Creates a vector containing information about all installed mods, stored in ModInfo
   * objects.
   * \return The vector.
   */
  std::vector<ModInfo> getModInfo() const;
  /*!
   * \brief Getter for the current mod load order of one Deployer.
   * \param deployer The target Deployer.
   * \return The load order.
   */
  std::shared_ptr<TreeItem<DeployerEntry>> getLoadorder(int deployer) const;
  /*!
   * \brief Getter for the path to the staging directory. This is where all installed
   * mods are stored.
   * \return The path.
   */
  const std::filesystem::path& getStagingDir() const;
  /*!
   * \brief Setter for the path to the staging directory. This is where all installed
   * mods are stored.
   * \param staging_dir The new staging directory path.
   * \param move_existing If true: Move all installed mods to the new directory.
   * \throws Json::LogicError Indicates a logic error, e.g. trying to convert "123" to a bool,
   * while parsing.
   * \throws Json::RuntimeError Indicates a syntax error in the JSON file.
   * \throws ParseError Indicates a semantic error while parsing the JSON file, e.g.
   * the active member of a group is not part of that group.
   */
  void setStagingDir(std::string staging_dir, bool move_existing);
  /*!
   * \brief Getter for the name of this application.
   * \return The name.
   */
  const std::string& name() const;
  /*!
   * \brief Setter for the name of this application.
   * \param newName The new name.
   */
  void setName(const std::string& newName);
  /*!
   * \brief Returns the number of Deployer objects for this application.
   * \return The number of Deployers.
   */
  int getNumDeployers() const;
  /*!
   * \brief Getter for the name of the file used to store this objects internal state.
   * \return The name.
   */
  const std::string& getConfigFileName() const;
  /*!
   * \brief Changes the name of an installed mod.
   * \param mod_id Id of the target mod.
   * \param new_name The new name.
   */
  void changeModName(int mod_id, const std::string& new_name);
  /*!
   * \brief Checks for file conflicts of given mod with all other mods in the load order for
   * one Deployer.
   * \param deployer The target Deployer
   * \param mod_id Mod to be checked.
   * \param show_disabled If true: Also check for conflicts with disabled mods.
   * \return A vector with information about conflicts with every other mod.
   */
  std::vector<ConflictInfo> getFileConflicts(int deployer, int mod_id, bool show_disabled) const;
  /*!
   * \brief Fills an AppInfo object with information about this object.
   * \return The AppInfo object.
   */
  AppInfo getAppInfo() const;
  /*!
   * \brief Adds a new tool to this application.
   * \param tool The new tool.
   */
  void addTool(const Tool& tool);
  /*!
   * \brief Removes a tool.
   * \param tool_id The tool's id.
   */
  void removeTool(int tool_id);
  /*!
   * \brief Getter for the tools of this application.
   * \return A vector of tools.
   */
  std::vector<Tool> getTools() const;
  /*!
   * \brief Getter for the command used to run this application.
   * \return The command.
   */
  const std::string& command() const;
  /*!
   * \brief Setter for the command used to run this application.
   * \param newCommand The new command.
   */
  void setCommand(const std::string& newCommand);
  /*!
   * \brief Used to set type, name and target directory for one deployer.
   * \param deployer Target Deployer.
   * \param info Contains all data needed to edit a deployer, e.g. its new name.
   */
  void editDeployer(int deployer, const EditDeployerInfo& info);
  /*!
   * \brief Checks for conflicts with other mods for one Deployer.
   * Two mods are conflicting if they share at least one file.
   * \param deployer Target Deployer.
   * \param mod_id The mod to be checked.
   * \return A set of mod ids which conflict with the given mod.
   */
  std::unordered_set<int> getModConflicts(int deployer, int mod_id);
  /*!
   * \brief Sets the currently active profile.
   * \param profile The new profile.
   */
  void setProfile(int profile);
  /*!
   * \brief Adds a new profile and optionally copies it's load order from an existing profile.
   * \param info Contains the data for the new profile.
   */
  void addProfile(const EditProfileInfo& info);
  /*!
   * \brief Removes a profile.
   * \param profile The profile to be removed.
   */
  void removeProfile(int profile);
  /*!
   * \brief Returns a vector containing the names of all profiles.
   * \return The vector.
   */
  std::vector<std::string> getProfileNames() const;
  /*!
   * \brief Used to set the name of a profile.
   * \param profile Target Profile
   * \param info Contains the new profile data.
   */
  void editProfile(int profile, const EditProfileInfo& info);
  /*!
   * \brief Used to replace an existing tool with a new tool.
   * \param tool_id Target tool to be replaced.
   * \param new_tool The new tool.
   */
  void editTool(int tool_id, const Tool& new_tool);
  /*!
   * \brief Checks if files can be deployed.
   * \return A tuple containing:
   * A return code: 0: No error, 1: Error while writing to a deployer's source directory,
   *  2: Error while creating a hard link, 3: Error while writing to a deployer's target directory.
   * The deployer's target path (if an error occured, else "").
   * A more detailed error message (if an error occured, else "").
   */
  std::tuple<int, std::string, std::string> verifyDeployerDirectories();
  /*!
   * \brief Adds a mod to an existing group and makes the mod the active member of that group.
   * \param mod_id The mod's id.
   * \param group The target group.
   * \param progress_node Used to inform about the current progress.
   */
  void addModToGroup(int mod_id, int group, std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Removes a mod from it's group.
   * \param mod_id Target mod.
   * \param update_conflicts If true: Update relevant conflict groups.
   * \param progress_node Used to inform about the current progress.
   */
  void removeModFromGroup(int mod_id,
                          bool update_conflicts = true,
                          std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Creates a new group containing the two given mods. A group is a set of mods
   * where only one member, the active member, will be deployed.
   * \param first_mod_id First mod. This will be the active member of the new group.
   * \param second_mod_id Second mod.
   * \param progress_node Used to inform about the current progress.
   */
  void createGroup(int first_mod_id,
                   int second_mod_id,
                   std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Changes the active member of given group to given mod.
   * \param group Target group.
   * \param mod_id The new active member.
   * \param progress_node Used to inform about the current progress.
   */
  void changeActiveGroupMember(int group,
                               int mod_id,
                               std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Sets the given mod's version to the given new version.
   * \param mod_id Target mod.
   * \param new_version The new version.
   */
  void changeModVersion(int mod_id, const std::string& new_version);
  /*!
   * \brief Returns the number of groups.
   * \return The number of groups.
   */
  int getNumGroups();
  /*!
   * \brief Checks if given mod belongs to any group.
   * \param mod_id Target mod.
   * \return True if mod belongs to a group, else: False.
   */
  bool modHasGroup(int mod_id);
  /*!
   * \brief Returns the group to which the given mod belongs.
   * \param mod_id Target mod.
   * \return The group, or -1 if the mod has no group.
   */
  int getModGroup(int mod_id);
  /*!
   * \brief Sorts the load order by grouping mods which contain conflicting files.
   * \param deployer Deployer for which the currently active load order is to be sorted.
   */
  void sortModsByConflicts(int deployer);
  /*!
   * \brief Returns the conflicts groups for the current profile of given deployer.
   * \param deployer Target Deployer.
   * \return The conflict info.
   */
  std::vector<std::vector<int>> getConflictGroups(int deployer);
  /*!
   * \brief Updates which \ref Deployer "deployer" should manage given mods.
   * \param mod_id Vector of mod ids to be added.
   * \param deployers Bool for every deployer, indicating if the mods should be managed
   * by that deployer.
   */
  void updateModDeployers(const std::vector<int>& mod_ids, const std::vector<bool>& deployers);
  /*! \brief Getter for icon_path_. */
  std::filesystem::path iconPath() const;
  /*!
   * \brief Setter for icon_path_.
   * \param icon_path The new icon path
   */
  void setIconPath(const std::filesystem::path& icon_path);

  /*!
   * \brief Verifies if reading/ writing to the staging directory is possible and if the
   * JSON file containing information about installed mods can be parsed.
   * \param staging_dir Path to the staging directory.
   * \return A code indicating success(0), an IO error(1) or an error during JSON parsing(2).
   */
  static int verifyStagingDir(std::filesystem::path staging_dir);
  /*!
   * \brief Creates DeployerInfo for one Deployer.
   * \param deployer Target deployer.
   */
  DeployerInfo getDeployerInfo(int deployer);
  /*! \brief Setter for log callback. */
  void setLog(const std::function<void(Log::LogLevel, const std::string&)>& newLog);
  /*!
   * \brief Adds a new target file or directory to be managed by the BackupManager.
   * \param path Path to the target file or directory.
   * \param name Display name for this target.
   * \param backup_names Display names for initial backups. Must contain at least one.
   */
  void addBackupTarget(const std::filesystem::path& path,
                       const std::string& name,
                       const std::vector<std::string>& backup_names);
  /*!
   * \brief Removes the given backup target by deleting all backups, except for the active one,
   * and all config files.
   * \param target_id Target to remove.
   */
  void removeBackupTarget(int target_id);
  /*!
   * \brief Removes all targets by deleting all backups, except for the active ones,
   * and all config files.
   */
  void removeAllBackupTargets();
  /*!
   * \brief Adds a new backup for the given target by copying the currently active backup.
   * \param target_id Target for which to create a new backup.
   * \param name Display name for the new backup.
   * \param source Backup from which to copy files to create the new backup. If -1:
   * copy currently active backup.
   */
  void addBackup(int target_id, const std::string& name, int source);
  /*!
   * \brief Deletes the given backup for given target.
   * \param target_id Target from which to delete a backup.
   * \param backup_id Backup to remove.
   */
  void removeBackup(int target_id, int backup_id);
  /*!
   * \brief Changes the currently active backup for the given target.
   * \param target_id Target for which to change the active backup.
   * \param backup_id New active backup.
   */
  void setActiveBackup(int target_id, int backup_id);
  /*!
   * \brief Returns a vector containing information about all managed backup targets.
   * \return The vector.
   */
  std::vector<BackupTarget> getBackupTargets() const;
  /*!
   * \brief Changes the name of the given backup for the given target
   * \param target_id Backup target.
   * \param backup_id Backup to be edited.
   * \param name The new name.
   */
  void setBackupName(int target_id, int backup_id, const std::string& name);
  /*!
   * \brief Changes the name of the given backup target
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
  /*! \brief Performs a cleanup for the previous installation. */
  void cleanupFailedInstallation();
  /*!
   * \brief Sets the callback function used to inform about the current task's progress.
   * \param progress_callback The function.
   */
  void setProgressCallback(const std::function<void(float)>& progress_callback);
  /*!
   * \brief Uninstalls all mods which are inactive group members of any group which contains
   * any of the given mods.
   * \param mod_ids Ids of the mods for which to uninstall group members.
   */
  void uninstallGroupMembers(const std::vector<int>& mod_ids);
  /*!
   * \brief Adds a new tag with the given name. Fails if a tag by that name already exists.
   * \param tag_name Name for the new tag.
   * \throw std::runtime_error If a tag by that name exists.
   */
  void addManualTag(const std::string& tag_name);
  /*!
   * \brief Removes the tag with the given name, if it exists.
   * \param tag_name Tag to be removed.
   * \param update_map If true: Update the manual tag map.
   */
  void removeManualTag(const std::string& tag_name, bool update_map = true);
  /*!
   * \brief Changes the name of the given tag to the given new name.
   * Fails if a tag by the given name exists.
   * \param old_name Name of the target tag.
   * \param new_name Target tags new name.
   * \param update_map If true: Update the manual tag map.
   * \throw std::runtime_error If a tag with the given new_name exists.
   */
  void changeManualTagName(const std::string& old_name,
                           const std::string& new_name,
                           bool update_map = true);
  /*!
   * \brief Adds the given tags to all given mods.
   * \param tag_name Target tags name.
   * \param mod_ids Target mod ids.
   */
  void addTagsToMods(const std::vector<std::string>& tag_names, const std::vector<int>& mod_ids);
  /*!
   * \brief Removes the given tags from the given mods.
   * \param tag_name Target tags name.
   * \param mod_ids Target mod ids.
   */
  void removeTagsFromMods(const std::vector<std::string>& tag_names,
                          const std::vector<int>& mod_ids);
  /*!
   * \brief Sets the tags for all given mods to the given tags.
   * \param tag_names Names of the new tags.
   * \param mod_ids Target mod ids.
   */
  void setTagsForMods(const std::vector<std::string>& tag_names, const std::vector<int> mod_ids);
  /*!
   * \brief Performs the given editing actions on the manual tags.
   * \param actions Editing actions.
   */
  void editManualTags(const std::vector<EditManualTagAction>& actions);
  /*!
   * \brief Adds a new auto tag.
   * \param name The new tags name.
   * \param expression Expression used for the new tags evaluator.
   * \param conditions Conditions used for the new tags evaluator.
   * \param update If true: Update the auto tag map and the settings.
   * \throw std::runtime_error If a tag by that name exists.
   */
  void addAutoTag(const std::string& tag_name,
                  const std::string& expression,
                  const std::vector<TagCondition>& conditions,
                  bool update);
  /*!
   * \brief Adds a new auto tag from the given Json object.
   * \param json_tag Json object representing the new auto tag.
   * \param update If true: Update the auto tag map and the settings.
   * \throw std::runtime_error If a tag by that name exists.
   */
  void addAutoTag(const Json::Value& json_tag, bool update);
  /*!
   * \brief Removes the given auto tag.
   * \param name Tag to be removed.
   * \param update If true: Update the auto tag map and the settings.
   */
  void removeAutoTag(const std::string& tag_name, bool update);
  /*!
   * \brief Changes the name of the given auto tag to the given new name.
   * Fails if a tag by the given name exists.
   * \param old_name Name of the target tag.
   * \param new_name Target tags new name.
   * \param update If true: Update the auto tag map.
   * \throw std::runtime_error If a tag with the given new_name exists.
   */
  void renameAutoTag(const std::string& old_name, const std::string& new_name, bool update);
  /*!
   * \brief Changes the given tags evaluator according to the given expression and conditions.
   * \param tag_name Target auto tag.
   * \param expression New expression to be used.
   * \param conditions Conditions for the new expression.
   * \param update If true: Update the auto tag map.
   */
  void changeAutoTagEvaluator(const std::string& tag_name,
                              const std::string& expression,
                              const std::vector<TagCondition>& conditions,
                              bool update);
  /*!
   * \brief Performs the given editing actions on the auto tags.
   * \param actions Editing actions.
   */
  void editAutoTags(const std::vector<EditAutoTagAction>& actions);
  /*! \brief Reapply all auto tags to all mods. */
  void reapplyAutoTags();
  /*!
   * \brief Reapplies auto tags to the specified mods.
   * \param mod_ids Mods to which auto tags are to be reapplied.
   */
  void updateAutoTags(const std::vector<int> mod_ids);
  /*! \brief Deletes all data for this app. */
  void deleteAllData();
  /*!
   * \brief Sets the app version of the currently active profile to the given version.
   * \param app_version The new app version.
   */
  void setAppVersion(const std::string& app_version);
  /*!
   * \brief Sets the given mods local and remote sources to the given paths.
   * \param mod_id Target mod id.
   * \param local_source Path to a local archive or directory used for mod installation.
   * \param remote_source Remote URL from which the mod was downloaded.
   */
  void setModSources(int mod_id, const std::string& local_source, const std::string& remote_source);
  /*!
   * \brief Fetches data from NexusMods for the given mod.
   * \param mod_id Target mod id.
   * \return A Mod object containing all data from NexusMods regarding that mod.
   */
  nexus::Page getNexusPage(int mod_id);
  /*! \brief Checks for updates for all mods. */
  void checkForModUpdates();
  /*!
   * \brief Checks for updates for mods with the given ids.
   * \param mod_ids Ids of the mods for which to check for updates.
   */
  void checkModsForUpdates(const std::vector<int>& mod_ids);
  /*!
   * \brief Temporarily disables update notifications for the given mods. This is done
   * by setting the mods remote_update_time to the installation_time.
   * \param mod_ids Ids of the mods for which update notifications are to be disabled.
   */
  void suppressUpdateNotification(const std::vector<int>& mod_ids);
  /*!
   * \brief Checks if files deployed by the given deployer have been externally overwritten.
   * \param deployer Deployer to check.
   * \return Contains data about overwritten files.
   */
  ExternalChangesInfo getExternalChanges(int deployer);
  /*!
   * \brief Currently only supports hard link deployment.
   * For every given file: Moves the modified file into the source mods directory and links
   * it back in, if the changes are to be kept. Else: Deletes that file and restores
   * the original link.
   * \param deployer Target deployer.
   * \param changes_to_keep Contains paths to modified files, the id of the mod currently
   * responsible for that file and a bool which indicates whether or not changes to
   * that file should be kept.
   */
  void keepOrRevertFileModifications(int deployer, const FileChangeChoices& changes_to_keep) const;
  /*! \brief For all deployers: If using hard links that can't be created, switch to sym links. */
  void fixInvalidHardLinkDeployers();
  /*!
   * \brief Exports configurations for the given deployers and the given auto tags to a json file.
   * Does not include mods.
   * \param deployers Deployers to export.
   * \param auto_tags Auto tags to export.
   */
  void exportConfiguration(const std::vector<int>& deployers,
                           const std::vector<std::string>& auto_tags);
  /*!
   * \brief Updates the file ignore list for ReverseDeployers.
   * \param deployer Target deployer.
   */
  void updateIgnoredFiles(int deployer);
  /*!
   * \brief Adds the given mod to the ignore list of the given ReverseDeployer.
   * \param deployer Target deployer.
   * \param mod_id Mod to be ignored.
   */
  void addModToIgnoreList(int deployer, int mod_id);
  /*!
   * \brief Applies the given mod action to the given mod.
   * \param deployer Target deployer.
   * \param action Action to be applied.
   * \param mod_id Target mod.
   */
  void applyModAction(int deployer, int action, int mod_id);
  /*!
   * \brief Returns the path used to store downloaded mods.
   * \return The download path.
   */
  std::filesystem::path getDownloadDir() const;

private:
  /*! \brief The subdirectory used to store downloads. */
  static inline constexpr std::string DOWNLOAD_DIR = "_download";

  /*! \brief The name of this application. */
  std::string name_;
  /*! \brief Contains the internal state of this object. */
  Json::Value json_settings_;
  /*! \brief The path to the staging directory containing all installed mods. */
  std::filesystem::path staging_dir_;
  /*! \brief Contains all currently installed mods. */
  std::vector<Mod> installed_mods_;
  /*! \brief Contains every Deployer used by this application. */
  std::vector<std::unique_ptr<Deployer>> deployers_;
  /*! \brief Contains all tools for this application. */
  std::vector<Tool> tools_;
  /*! \brief The command used to run this application. */
  std::string command_ = "";
  /*! \brief The currently active profile id. */
  int current_profile_ = 0;
  /*! \brief Contains names of all profiles. */
  std::vector<std::string> profile_names_;
  /*! \brief For every group: A vector containing every mod in that group. */
  std::vector<std::vector<int>> groups_;
  /*! \brief Maps mods to their groups. */
  std::map<int, int> group_map_;
  /*! \brief Contains the active member of every group. */
  std::vector<int> active_group_members_;
  /*! \brief Maps mods to the installer used during their installation. */
  std::map<int, std::string> installer_map_;
  /*! \brief Path to this applications icon. */
  std::filesystem::path icon_path_;
  /*! \brief Callback for logging. */
  std::function<void(Log::LogLevel, const std::string&)> log_ = [](Log::LogLevel a,
                                                                   const std::string& b) {};
  /*! \brief Manages all backups for this application. */
  BackupManager bak_man_;
  /*! \brief Id of the most recently installed mod. */
  int last_mod_id_ = -1;
  /*! \brief Contains all known manually managed tags. */
  std::vector<ManualTag> manual_tags_;
  /*! \brief Maps mod ids to a vector of manual tags associated with that mod. */
  std::map<int, std::vector<std::string>> manual_tag_map_;
  /*! \brief Contains all known auto tags. */
  std::vector<AutoTag> auto_tags_;
  /*! \brief Maps mod ids to a vector of auto tags associated with that mod. */
  std::map<int, std::vector<std::string>> auto_tag_map_;
  /*!
   *  \brief For every profile: The version of the app managed by that profile.
   *
   *  This does not refer to a ModdedApplication object but rather the actually
   *  modded application.
   */
  std::vector<std::string> app_versions_;
  /*! \brief Callback used to inform about the current task's progress. */
  std::function<void(float)> progress_callback_ = [](float f) {};
  /*! \brief File name used to store exported deployers and auto tags. */
  std::string export_file_name = "exported_config";
  /*! \brief Steam app id. Or -1 if not a Steam app. */
  long steam_app_id_;

  /*!
   * \brief Updates json_settings_ with the current state of this object.
   * \param write If true: write json_settings_ to a file after updating.
   */
  void updateSettings(bool write = false);
  /*!
   * \brief Writes json_settings_ to a file at app_mod_dir_/CONFIG_FILE_NAME.
   */
  void writeSettings() const;
  /*!
   * \brief Reads json_settings_ from a file at app_mod_dir_/CONFIG_FILE_NAME.
   */
  void readSettings();
  /*!
   * \brief Updates the internal state of this object to the state stored in json_settings_.
   * \param read If true: Read json_settings_ from a file before updating.
   */
  void updateState(bool read = false);
  /*!
   * \brief Returns the name of a mod.
   * \param mod_id The mod.
   * \return The name.
   * \throws Json::LogicError Indicates a logic error, e.g. trying to convert "123" to a bool,
   * while parsing.
   * \throws Json::RuntimeError Indicates a syntax error in the JSON file.
   * \throws ParseError Indicates a semantic error while parsing the JSON file, e.g.
   * the active member of a group is not part of that group.
   */
  std::string getModName(int mod_id) const;
  /*!
   * \brief Updates the load order for every Deployer to reflect the current mod groups.
   * \param progress_node Used to inform about the current progress.
   */
  void updateDeployerGroups(std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief If given mod contains a sub-directory managed by a deployer that is not the given
   * deployer, creates a new mod which contains that sub-directory.
   * \param mod_id Mod to check.
   * \param deployer Deployer which currently manages the given mod.
   */
  void splitMod(int mod_id, int deployer);
  /*!
   * \brief Replaces an existing mod with the mod specified by the given argument.
   * \param info Contains all data needed to install the mod.
   */
  void replaceMod(const ImportModInfo& info);
  /*! \brief Updates manual_tag_map_ with the information contained in manual_tags_. */
  void updateManualTagMap();
  /*! \brief Updates auto_tag_map_ with the information contained in auto_tags_. */
  void updateAutoTagMap();
  /*!
   * \brief Checks for available updates for mods with the given index in installed_mods_.
   * \param target_mod_indices Target mod indices.
   */
  void performUpdateCheck(const std::vector<int>& target_mod_indices);
  /*!
   * \brief Checks if the given path belongs to a steam installation or prefix directory.
   * Replaces installation or prefix path components with tokens.
   * \param path Path to check.
   * \return If no steam paths are found: The input path, else: The modified path.
   */
  std::string generalizeSteamPath(const std::string& path);
  /*! \brief If the icon path is a steam path: Update it to the new format. */
  void updateSteamIconPath();
  /*! \brief If steam_app_id_ == -1: Try to determine the app id. */
  void updateSteamAppId();
};
