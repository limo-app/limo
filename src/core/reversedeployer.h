/*!
 * \file reversedeployer.h
 * \brief Header for the ReverseDeployer class.
 */

#pragma once

#include "deployer.h"


/*!
 * \brief Moves all files not managed by another deployer out of the target
 * directory and links them back in.
 */
class ReverseDeployer : public Deployer
{
public:
  /*!
   * \brief Constructor.
   * \param source_path Path to directory containing mods installed using the Installer class.
   * \param dest_path Path to target directory for mod deployment.
   * \param name A custom name for this instance.
   * \param deploy_mode Determines how files are deployed to the target directory.
   * \param separate_profile_dirs If true: Store files on a per profile basis.
   * Else: All profiles use the same files.
   * \param update_ignore_list If true: Add all files in dest_path not managed by another
   * deployer to the ignore list.
   */
  ReverseDeployer(const std::filesystem::path& source_path,
                  const std::filesystem::path& dest_path,
                  const std::string& name,
                  DeployMode deploy_mode = hard_link,
                  bool separate_profile_dirs = false,
                  bool update_ignore_list = false);

  /*!
   *  \brief Scans the target directory for files not managed by other deployers.
   *  \param write If true: Write managed files to file.
   *  \param progress_node Used to inform about progress.
   */
  void updateManagedFiles(bool write = false, std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Updates all managed files and links / unlinks files depending on whether or not
   * they are disabled.
   * \param progress_node Used to inform about the current progress of deployment.
   * \return Since this is an autonomous deployer, the returned map is always empty.
   */
  std::map<int, unsigned long> deploy(std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Updates all managed files and links / unlinks files depending on whether or not
   * they are disabled.
   * \param loadorder Ignored.
   * \param progress_node Used to inform about the current progress of deployment.
   * \return Since this is an autonomous deployer, the returned map is always empty.
   */
  std::map<int, unsigned long> deploy(const std::vector<int>& loadorder,
                                      std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Unlinks all managed files
   * \param progress_node Used to inform about the current progress.
   */
  virtual void unDeploy(std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Moves a mod from one position in the load order to another. Saves changes to disk.
   * \param from_index Index of mod to be moved.
   * \param to_index Destination index.
   */
  void swapChild(int from_index, int to_index) override;
  /*!
   * \brief Enables or disables the given mod in the load order. Saves changes to disk.
   * \param mod_id Mod to be edited.
   * \param status The new status.
   */
  void setModStatus(int mod_id, bool status) override;
  /*!
   * \brief Conflict groups are not supported by this type.
   * \return All plugins in the non conflicting group.
   */
  std::vector<std::vector<int>> getConflictGroups() const override;
  /*!
   * \brief Returns the vector of managed files.
   * \return The vector.
   */
  std::vector<std::string> getModNames() const override;
  /*!
   * \brief Adds a new profile and optionally copies it's load order from an existing profile.
   * Profiles are stored in the deactivated list file.
   * \param source The profile to be copied. A value of -1 indicates no copy.
   */
  void addProfile(int source = -1) override;
  /*!
   * \brief Removes a profile.
   * \param profile The profile to be removed.
   */
  void removeProfile(int profile) override;
  /*!
   * \brief Sets the current profile to the given profile. Updates the current loadorder.
   * \param profile The new profile.
   */
  void setProfile(int profile) override;
  /*!
   * \brief Not supported by this type.
   * \param newConflict_groups Ignored.
   */
  void setConflictGroups(const std::vector<std::vector<int>>& newConflict_groups) override;
  /*!
   * \brief Returns the number of managed files.
   * \return The number of plugins.
   */
  int getNumMods() override;
  /*!
   * \brief Getter for the current plugin load order.
   * \return The load order.
   */
  std::shared_ptr<TreeItem<DeployerEntry>> getLoadorder() override;
  /*!
   * \brief Does nothing since this deployer manages its own mods.
   * \param mod_id Ignored.
   * \param enabled Ignored.
   * \param update_conflicts Ignored.
   * \return False.
   */
  bool addMod(int mod_id, bool enabled = true, bool update_conflicts = true) override;
  /*!
   * \brief Not supported by this type.
   * \param mod_id Ignored.
   * \return False.
   */
  bool removeMod(int mod_id) override;
  /*!
   * \brief Since this deployer uses its own internal mod ids, this function always
   * returns false.
   * \param mod_id Ignores
   * \return False.
   */
  bool hasMod(int mod_id) override;
  /*!
   * \brief Does nothing since this deployer manages its own mods.
   * \param old_id Ignored.
   * \param new_id Ignored
   * \return False.
   */
  bool swapMod(int old_id, int new_id) override;
  /*!
   * \brief Not supported.
   * \param mod_id Ignored.
   * \param show_disabled Ignored.
   * \param progress_node Set to 100%.
   * \return An empty vector.
   */
  std::vector<ConflictInfo> getFileConflicts(
    int mod_id,
    bool show_disabled = false,
    std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Checks for conflicts with other mods.
   * Two mods are conflicting if they share at least one record.
   * \param mod_id The mod to be checked.
   * \param progress_node Used to inform about the current progress.
   * \return A set of mod ids which conflict with the given mod.
   */
  std::unordered_set<int> getModConflicts(int mod_id,
                                          std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Sorting is not supported.
   * \param progress_node Used to inform about the current progress.
   */
  void sortModsByConflicts(std::optional<ProgressNode*> progress_node = {}) override;
  /*! \brief Moves all files back to their original location. */
  void cleanup() override;
  /*!
   * \brief Searches the load order for the given mod id and returns the corresponding mods
   * activation status, if found.
   * \param mod_id Mod to be found.
   * \return The activation status, if found.
   */
  std::optional<bool> getModStatus(int mod_id);
  /*!
   * \brief Getter for mod tags.
   * \return For every mod: A vector of auto tags added to that mod.
   */
  virtual std::vector<std::vector<std::string>> getAutoTags() override;
  /*!
   * \brief Returns all available auto tag names.
   * \return The tag names.
   */
  virtual std::map<std::string, int> getAutoTagMap() override;
  /*!
   * \brief Iterates over all deployed files in the source and target directory
   * and checks if they have been deleted externally.
   * \param progress_node Used to inform about progress.
   * \return A vector containing pairs of paths to deleted files and the corresponding mod id.
   */
  virtual std::vector<std::pair<std::filesystem::path, int>> getExternallyModifiedFiles(
    std::optional<ProgressNode*> progress_node = {}) const override;
  /*!
   * \brief For all given files: If change is to be kept: Delete the file, else: Restore the file.
   * \param changes_to_keep Contains vectors of relative file paths and decision of whether or not
   * to keep the change for that file.
   */
  virtual void keepOrRevertFileModifications(const FileChangeChoices& changes_to_keep) override;
  /*!
   * \brief This is not supported for this deployer type.
   * \param mod_id Ignored.
   * \param progress_node Ignored.
   */
  virtual void updateDeployedFilesForMod(
    int mod_id,
    std::optional<ProgressNode*> progress_node = {}) const override;
  /*!
   * \brief Adds all files currently in the target directory and not managed by another deployer
   * to the list of ignored files.
   * \param write If true: Write new list to disk.
   */
  void updateIgnoredFiles(bool write = false);
  /*! \brief Removes all currently ignored files from the list. */
  void deleteIgnoredFiles();
  /*!
   * \brief Returns a vector containing the ignored files.
   * \return The vector.
   */
  std::vector<std::string> getIgnoredFiles() const;
  /*!
   * \brief Enables/ disables separate managed files directories for every profile.
   *
   * When switched to enabled: Moves the current files to the directory of the active profile.
   * Creates empty directories for every inactive profile.
   * When switched to disabled: Uses files from the currently active profile and
   * deletes files for all other profiles.
   * \param enabled The new status.
   */
  void enableSeparateDirs(bool enabled);
  /*!
   * \brief Returns whether or not each profile uses a separate directory to store files.
   * \return True when separate directories are used.
   */
  bool usesSeparateDirs() const;
  /*!
   * \brief Returns the number of files in the ignore list.
   * \return The number of files.
   */
  int getNumIgnoredFiles() const;
  /*!
   * \brief Returns the number of profiles.
   * \return The number of profiles.
   */
  int getNumProfiles() const;
  /*!
   * \brief Returns the order in which the deploy function of different
   *  deployers should be called.
   * \return The priority.
   */
  virtual int getDeployPriority() const override;
  /*!
   * \brief Returns whether or not this deployer type supports sorting mods.
   * \return True if supported.
   */
  virtual bool supportsSorting() const override;
  /*!
   * \brief Returns whether or not this deployer type supports reordering mods.
   * \return True if supported.
   */
  virtual bool supportsReordering() const override;
  /*!
   * \brief Returns whether or not this deployer type supports showing mod conflicts.
   * \return True if supported.
   */
  virtual bool supportsModConflicts() const override;
  /*!
   * \brief Returns whether or not this deployer type supports showing file conflicts.
   * \return True if supported.
   */
  virtual bool supportsFileConflicts() const override;
  /*!
   * \brief Returns whether or not this deployer type supports browsing mod files.
   * \return True if supported.
   */
  virtual bool supportsFileBrowsing() const override;
  /*!
   * \brief Adds the file matching the given position in the current loadorder to the ignore list.
   * \param mod_id Position in the current loadorder.
   */
  void addModToIgnoreList(int mod_id);
  /*!
   * \brief Returns a vector containing valid mod actions.
   * \return For every mod: IDs of every valid mod_action which is valid for that mod.
   */
  virtual std::vector<std::vector<int>> getValidModActions() const override;

private:
  /*! \brief Name of the file containing paths of ignored files. */
  const std::string ignore_list_file_name_ = ".revdepl-ignored_files.json";
  /*! \brief Name of the file containing file paths and activation status for every profile. */
  const std::string managed_files_name_ = ".revdepl-managed_files.json";
  /*! \brief Name of the file containing the currently deployed load order. */
  const std::string deployed_loadorder_name_ = ".revdepl-deployed_files.json";
  /*! \brief For every profile: A vector containing every file that is not to be deployed. */
  std::vector<std::map<std::filesystem::path, bool>> managed_files_;
  /*! \brief Contains all files and their enabled status for the current load order. */
  std::vector<std::pair<std::filesystem::path, bool>> current_loadorder_;
  /*! \brief Contains all files and their enabled status for the currently deployed load order. */
  std::vector<std::pair<std::filesystem::path, bool>> deployed_loadorder_;
  /*! \brief Contains all files which should be ignored by this deployer. */
  std::unordered_set<std::string> ignored_files_;
  /*! \brief Currently deployed profile. */
  int deployed_profile_ = -1;
  /*! \brief If true: Store files on a per profile basis. Else: All profiles use the same files. */
  bool separate_profile_dirs_ = false;
  /*!
   *  \brief Determines the order in which the deploy function of different
   *  deployers should be called
   */
  const int deploy_priority_ = 2;
  /*! \brief The total number of files in the target directory during previous deployment. */
  int number_of_files_in_target_ = 0;

  /*! \brief Reads a list of ignored files from the ignore list file. */
  void readIgnoredFiles();
  /*! \brief Writes the list of ignored files to disk. */
  void writeIgnoredFiles() const;
  /*! \brief Reads all files for every profile from a file in source_path_. */
  void readManagedFiles();
  /*! \brief Writes all files for every profile to a file in source_path_. */
  void writeManagedFiles() const;
  /*!
   * \brief Recursively adds all files not ignored or handled by other deployers in
   * dir to profile_files_ for the current profile.
   * \param target_dir Directory in which to search for files.
   * \param deployed_files Contains relative paths to all files deployed by another deployer.
   * \param current_deployer_path Target directory of another deployer managing this directory.
   * \param update_ignored_files If true: Update the list of ignored files instead.
   * \param progress_node Used to inform about progress.
   */
  int updateFilesInDir(const std::filesystem::path& target_dir,
                       const std::unordered_set<std::filesystem::path>& deployed_files,
                       std::filesystem::path current_deployer_path,
                       bool update_ignored_files = false,
                       std::optional<ProgressNode*> progress_node = {});
  /*! \brief Moves all managed files from dest_path_ to source_path_. */
  void moveFilesFromTargetToSource() const;
  /*! \brief Updates current_loadorder_ to reflect managed_files_[current_profile_]. */
  void updateCurrentLoadorder();
  /*! \brief Uses the operation specified in deploy mode to copy/ link files from source to target.*/
  void deployManagedFiles();
  /*!
   * \brief Returns the full path pointing to the given file in source_path_.
   * \param path Relative path to to convert.
   * \param profile The profile for which to determine the path.
   * \return The full path.
   */
  std::filesystem::path getSourcePath(const std::filesystem::path& path, int profile) const;
  /*!
   * \brief Deletes the given file from disk and the given profile. If separate directories are NOT used:
   * Deletes the file from all profiles.
   * Does NOT write manged files or update the current loadorder.
   * \param path Relative path to the file which is to be delete.
   * \param profile Profile from which to delete the file.
   */
  void deleteFile(const std::filesystem::path& path, int profile);
};
