/*!
 * \file deployer.h
 * \brief Header for the Deployer class.
 */

#pragma once

#include "conflictinfo.h"
#include "deployerentry.hpp"
#include "treeitem.h"
#include "filechangechoices.h"
#include "log.h"
#include "progressnode.h"
#include <filesystem>
#include <map>
#include <optional>
#include <unordered_set>
#include <vector>


/*!
 * \brief Handles deployment of mods to target directory.
 */
class Deployer
{
public:
  /*! \brief Describes how files should be deployed to the target directory. */
  enum DeployMode
  {
    /*! \brief Create hard links for files. */
    hard_link = 0,
    /*! \brief Create sym links for files. */
    sym_link = 1,
    /*! \brief Copy files. */
    copy = 2
  };

  /*!
   * \brief Constructor.
   * \param source_path Path to directory containing mods installed using the Installer class.
   * \param dest_path Path to target directory for mod deployment.
   * \param name A custom name for this instance.
   * \param deploy_mode Determines how files are deployed to the target directory.
   */
  Deployer(const std::filesystem::path& source_path,
           const std::filesystem::path& dest_path,
           const std::string& name,
           DeployMode deploy_mode = hard_link);

  /*!
   * \brief Getter for path to deployment target directory.
   * \return The path.
   */
  std::string getDestPath() const;
  /*!
   * \brief Getter for the path to the deployer's source directory.
   * \return The path.
   */
  std::string getSourcePath() const;
  /*!
   * \brief Getter for deployer name.
   * \return The name.
   */
  std::string getName() const;
  /*!
   * \brief Setter for deployer name.
   * \param name The new name.
   */
  void setName(const std::string& name);
  /*!
   * \brief Deploys all mods to the target directory using hard links.
   * If any file already exists in the target directory, a backup for that file is created.
   * Previously backed up files are automatically restored if no mod in the current load order
   * overwrites them. Conflicts are handled by overwriting mods earlier in the load order
   * with later mods.
   * \param loadorder A vector of mod ids representing the load order.
   * \param progress_node Used to inform about the current progress of deployment.
   * \return A map from deployed mod ids to their respective mods total size on disk.
   */
  virtual std::map<int, unsigned long> deploy(const std::vector<int>& loadorder,
                                              std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Deploys all mods to the target directory using hard links.
   * If any file already exists in the target directory, a backup for that file is created.
   * Previously backed up files are automatically restored if no mod in the current load order
   * overwrites them. Conflicts are handled by overwriting mods earlier in the load order
   * with later mods. This function uses the internal load order.
   * \param progress_node Used to inform about the current progress of deployment.
   * \return A map from deployed mod ids to their respective mods total size on disk.
   */
  virtual std::map<int, unsigned long> deploy(std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Removes all deployed mods from the target directory and restores backups.
   * \param progress_node Used to inform about the current progress.
   */
  virtual void unDeploy(std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Setter for the load order used for deployment.
   * \param loadorder The new load order.
   */
  void setLoadorder(const std::shared_ptr<TreeItem<DeployerEntry>> loadorder);
  void setLoadorder(Json::Value loadorder);
  /*!
   * \brief Getter for the current mod load order.
   * \return The load order.
   */
  virtual std::shared_ptr<TreeItem<DeployerEntry>> getLoadorder();
  /*!
   * \brief Returns the type of this deployer, i.e. SIMPLEDEPLOYER
   * \return The type.
   */
  std::string getType() const;
  /*!
   * \brief Moves a mod from one position in the load order to another.
   * \param from_index Index of mod to be moved.
   * \param to_index Destination index.
   */
  virtual void swapChild(int from_index, int to_index);
  virtual void swapNodes(std::shared_ptr<TreeItem<DeployerEntry>> nodeA, std::shared_ptr<TreeItem<DeployerEntry>> nodeB);
  /*!
   * \brief Appends a new mod to the load order.
   * \param mod_id Id of the mod to be added.
   * \param enabled Controls if the new mod will be enabled.
   * \param update_conflicts If true: Update mod conflict groups.
   * \return True iff the mod has been added.
   */
  virtual bool addMod(int mod_id, bool enabled = true, bool update_conflicts = true);
  /*!
   * \brief Removes a mod from the load order.
   * \param mod_id Id of the mod to be removed.
   * \return True iff the mod has been removed.
   */
  virtual bool removeNode(void *node_ptr);
  virtual bool removeMod(int mod_id);
  /*!
   * \brief Enables or disables the given mod in the load order.
   * \param mod_id Mod to be edited.
   * \param status The new status.
   */
  virtual void setModStatus(int mod_id, bool status);
  /*!
   * \brief Checks if given mod id is part of the load order.
   * \param mod_id Mod to be checked.
   * \return True is mod is in load order, else false.
   */
  virtual bool hasMod(int mod_id);
  /*!
   * \brief Checks for file conflicts of given mod with all other mods in the load order.
   * \param mod_id Mod to be checked.
   * \param show_disabled If true: Also check for conflicts with disabled mods.
   * \param progress_node Used to inform about the current progress.
   * \return A vector with information about conflicts with every other mod.
   */
  virtual std::vector<ConflictInfo> getFileConflicts(
    int mod_id,
    bool show_disabled = false,
    std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Returns the number of mods in the load order.
   * \return The number of mods.
   */
  virtual int getNumMods();
  /*!
   * \brief Getter for path to deployment target directory.
   * \return The path.
   */
  const std::filesystem::path& destPath() const;
  /*!
   * \brief Setter for path to deployment target directory.
   * \param newDest_path the new path.
   */
  void setDestPath(const std::filesystem::path& path);
  /*!
   * \brief Checks for conflicts with other mods.
   * Two mods are conflicting if they share at least one file.
   * \param mod_id The mod to be checked.
   * \param progress_node Used to inform about the current progress.
   * \return A set of mod ids which conflict with the given mod.
   */
  virtual std::unordered_set<int> getModConflicts(int mod_id,
                                                  std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Adds a new profile and optionally copies it's load order from an existing profile.
   * \param source The profile to be copied. A value of -1 indicates no copy.
   */
  virtual void addProfile(int source = -1);
  /*!
   * \brief Removes a profile.
   * \param profile The profile to be removed.
   */
  virtual void removeProfile(int profile);
  /*!
   * \brief Setter for the active profile.
   * \param profile The new profile.
   */
  virtual void setProfile(int profile);
  /*!
   * \brief Getter for the active profile.
   * \return The profile.
   */
  int getProfile() const;
  /*!
   * \brief Checks if writing to the deployment directory is possible.
   * \return A pair containing:
   * A code indicating success(0), an IO error(1) or an error during link creation(2).
   * A string with additional debug information.
   */
  std::pair<int, std::string> verifyDirectories();
  /*!
   * \brief Replaces the given id in the load order with a new id.
   * \param old_id The mod to be replaced.
   * \param new_id The new mod.
   * \return True iff the mod has been swapped.
   */
  virtual bool swapMod(int old_id, int new_id);
  /*!
   * \brief Sorts the load order by grouping mods which contain conflicting files.
   * \param progress_node Used to inform about the current progress.
   */
  virtual void sortModsByConflicts(std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Getter for the conflict groups of the current profile.
   * \return The conflict groups.
   */
  virtual std::vector<std::vector<int>> getConflictGroups() const;
  /*!
   * \brief Setter for the conflict groups of the current profile.
   * \param newConflict_groups The new conflict groups.
   */
  virtual void setConflictGroups(const std::vector<std::vector<int>>& newConflict_groups);
  /*!
   * \brief Getter for the current DeployMode.
   * \return The current DeployMode.
   */
  DeployMode getDeployMode() const;
  /*!
   * \brief Sets the current DeployMode.
   * \param deploy_mode The new DeployMode.
   */
  virtual void setDeployMode(DeployMode deploy_mode);
  /*! \brief Getter for is_autonomous_. */
  bool isAutonomous();
  /*!
   *  \brief Autonomous deployers override this tho provide names for their mods.
   *  Non Autonomous deployers return an empty vector.
   *  \return The mod name vector.
   */
  virtual std::vector<std::string> getModNames() const;
  /*! \brief Getter for mod source path. */
  std::filesystem::path sourcePath() const;
  /*!
   * \brief Setter for mod source path.
   * \param New source path.
   */
  void setSourcePath(const std::filesystem::path& newSourcePath);
  /*!
   * \brief Setter for log callback.
   * \param newLog New log callback
   */
  void setLog(const std::function<void(Log::LogLevel, const std::string&)>& newLog);
  /*!
   * \brief Removes all deployed mods from the target directory and deletes the file
   * which stores the state of this deployer.
   */
  virtual void cleanup();
  /*!
   * \brief Updates conflict_groups_ for the current profile.
   * \param progress_node Used to inform about the current progress.
   */
  void updateConflictGroups(std::optional<ProgressNode*> progress_node = {});
  /*! \brief Getter for \ref auto_update_conflict_groups_. */
  bool autoUpdateConflictGroups() const;
  /*! \brief Setter for \ref auto_update_conflict_groups_. */
  void setAutoUpdateConflictGroups(bool status);
  /*!
   * \brief Searches the load order for the given mod id and returns the corresponding mods
   * activation status, if found.
   * \param mod_id Mod to be found.
   * \return The activation status, if found.
   */
  std::optional<bool> getModStatus(int mod_id);
  /*!
   * \brief Getter for auto tags.
   * Only implemented in autonomous deployers.
   * \return For every mod: A vector of auto tags added to that mod.
   */
  virtual std::vector<std::vector<std::string>> getAutoTags();
  /*!
   * \brief Returns all available auto tag names mapped to the number of mods for that tag.
   * Only implemented in autonomous deployers.
   * \return The tag map.
   */
  virtual std::map<std::string, int> getAutoTagMap();
  /*!
   * \brief Currently only supports hard link deployment.
   * Checks if hard links of deployed files have been overwritten with new files.
   * \param progress_node Used to inform about the current progress.
   * \return Path to every file that has been created via hard linking a mod and later overwritten
   * externally and the id of the mod currently responsible for that file.
   */
  virtual std::vector<std::pair<std::filesystem::path, int>> getExternallyModifiedFiles(
    std::optional<ProgressNode*> progress_node = {}) const;
  /*!
   * \brief Currently only supports hard link deployment.
   * For every given file: Moves the modified file into the source mods directory and links
   * it back in, if the changes are to be kept. Else: Deletes that file and restores
   * the original link.
   * \param changes_to_keep Contains paths to modified files, the id of the mod currently
   * responsible for that file and a bool which indicates whether or not changes to
   * that file should be kept.
   */
  virtual void keepOrRevertFileModifications(const FileChangeChoices& changes_to_keep);
  /*!
   * \brief Updates the deployed files for one mod to match those in the mod's source directory.
   * \param mod_id Target mod.
   * \param progress_node Used to inform about progress.
   */
  virtual void updateDeployedFilesForMod(int mod_id,
                                         std::optional<ProgressNode*> progress_node = {}) const;
  /*! \brief If using hard_link deploy mode and links cannot be created: Switch to sym links. */
  virtual void fixInvalidLinkDeployMode();
  /*!
   * \brief Returns the order in which the deploy function of different
   *  deployers should be called.
   * \return The priority.
   */
  virtual int getDeployPriority() const;
  /*!
   * \brief Returns whether or not this deployer type supports sorting mods.
   * \return True if supported.
   */
  virtual bool supportsSorting() const;
  /*!
   * \brief Returns whether or not this deployer type supports reordering mods.
   * \return True if supported.
   */
  virtual bool supportsReordering() const;
  /*!
   * \brief Returns whether or not this deployer type supports showing mod conflicts.
   * \return True if supported.
   */
  virtual bool supportsModConflicts() const;
  /*!
   * \brief Returns whether or not this deployer type supports showing file conflicts.
   * \return True if supported.
   */
  virtual bool supportsFileConflicts() const;
  /*!
   * \brief Returns whether or not this deployer type supports browsing mod files.
   * \return True if supported.
   */
  virtual bool supportsFileBrowsing() const;
  /*!
   * \brief Returns whether or not this deployer type supports expandable items.
   * \return True if supported.
   */
  virtual bool supportsExpandableItems() const;
  /*!
   * \brief Returns whether or not this deployer type uses mod ids as references to
   * source mods. This is usually done by autonomous deployers.
   * \return False
   */
  virtual bool idsAreSourceReferences() const;
  /*!
   * \brief Returns names and icon names for additional actions which can be applied to a mod.
   * \return The actions.
   */
  virtual std::vector<std::pair<std::string, std::string>> getModActions() const;
  /*!
   * \brief Returns a vector containing valid mod actions.
   * \return For every mod: IDs of every valid mod_action which is valid for that mod.
   */
  virtual std::vector<std::vector<int>> getValidModActions() const;
  /*!
   * \brief Applies the given mod action to the given mod.
   * \param action Action to be applied.
   * \param mod_id Target mod.
   */
  virtual void applyModAction(int action, int mod_id);
  /*!
   * \brief Returns whether or not this deployer type is case invariant.
   * \return False.
   */
  virtual bool isCaseInvariant() const;
  /*!
   * \brief Returns whether sorting mods is allowed affect overwrite behavior.
   *
   * If this is set to false, sorting will always be safe and only affect how mods are displayed.
   * \return The safe sorting state.
   */
  bool getEnableUnsafeSorting() const;
  /*!
   * \brief Sets whether sorting mods is allowed affect overwrite behavior.
   *
   * If this is set to false, sorting will always be safe and only affect how mods are displayed.
   * \param The new safe sorting state.
   */
  void setEnableUnsafeSorting(bool enable);

protected:
  /*! \brief Type of this deployer, e.g. Simple Deployer. */
  std::string type_ = "Simple Deployer";
  /*! \brief Path to the directory containing all mods which are to be deployed. */
  std::filesystem::path source_path_;
  /*! \brief Path to the directory where all mods are deployed to. */
  std::filesystem::path dest_path_;
  /*! \brief The file extension appended to backed up files. */
  const std::string backup_extension_ = ".lmmbak";
  /*! \brief The file name for a file in the target directory containing names of deployed files*/
  const std::string deployed_files_name_ = ".lmmfiles";
  /*! \brief Name of the file indicating that the directory is managed by a deployer. */
  const std::string managed_dir_file_name_ = ".lmm_managed_dir";
  /*! \brief The name of this deployer. */
  std::string name_;
  /*! \brief The currently active profile. */
  int current_profile_ = 0;
  /*! \brief One load order per profile consisting of tuples of mod ids and their enabled status. */
  std::vector<std::shared_ptr<TreeItem<DeployerEntry>>> loadorders_;
  /*!
   * \brief For every profile: Groups of mods which conflict with each other. The last
   * group contains mods with no conflicts.
   */
  std::vector<std::vector<std::vector<int>>> conflict_groups_;
  /*! \brief Determines how files should be deployed to the target directory. */
  DeployMode deploy_mode_ = hard_link;
  /*! \brief Autonomous deployers manage their own mods and do not rely on ModdedApplication. */
  bool is_autonomous_ = false;
  /*! \brief If true: Automatically update conflict groups when necessary. */
  bool auto_update_conflict_groups_ = false;
  /*! \brief Determines whether sorting mods can affect overwrite behavior. */
  bool enable_unsafe_sorting_ = false;

  /*!
   * \brief Creates a pair of maps. One maps relative file paths to the mod id from which that
   * file is to be deployed. The other maps mod ids to their total file size on disk.
   * \param loadorder The load order used for file checks.
   * \return The generated maps.
   */
  std::pair<std::map<std::filesystem::path, int>, std::map<int, unsigned long>>
  getDeploymentSourceFilesAndModSizes(const std::vector<int>& loadorder) const;
  /*!
   * \brief Backs up all files which would be overwritten during deployment and restores all
   * files backed up during previous deployments files which are no longer overwritten.
   * \param source_files A map of files to be deployed to their source mods.
   * \param dest_files A map of files currently deployed to their source mods.
   */
  void backupOrRestoreFiles(const std::map<std::filesystem::path, int>& source_files,
                            const std::map<std::filesystem::path, int>& dest_files) const;
  /*!
   * \brief Hard links all given files to target directory.
   * \param source_files A map of files to be deployed to their source mods.
   * \param progress_node Used to inform about the current progress of deployment.
   */
  void deployFiles(const std::map<std::filesystem::path, int>& source_files,
                   std::optional<ProgressNode*> progress_node = {}) const;
  /*!
   * \brief Creates a map of currently deployed files to their source mods.
   * \param progress_node Used to inform about the current progress.
   * \param dest_path Directory containing the file in which deployed file names are stored.
   * If empty: Use the location in dest_path_ instead.
   * \return The map.
   */
  std::map<std::filesystem::path, int> loadDeployedFiles(
    std::optional<ProgressNode*> progress_node = {}, std::filesystem::path dest_path = "") const;
  /*!
   * \brief Creates a file containing information about currently deployed files.
   * \param deployed_files The currently deployed files.
   * \param progress_node Used to inform about the current progress.
   */
  void saveDeployedFiles(const std::map<std::filesystem::path, int>& deployed_files,
                         std::optional<ProgressNode*> progress_node = {}) const;
  /*!
   * \brief Creates a vector containing every file contained in one mod. Files are
   * represented as paths relative to the mods root directory.
   * \param mod_id Target mod.
   * \param include_directories If true: Also include all directories in the mod.
   * \return The vector of files.
   */
  std::vector<std::string> getModFiles(int mod_id, bool include_directories = false) const;
  /*! \brief Callback for logging. */
  std::function<void(Log::LogLevel, const std::string&)> log_ = [](Log::LogLevel a,
                                                                   const std::string& b) {};
  /*!
   * \brief modPathExists Checks if the directory containing the given mod exists.
   * \param mod_id If of the mod to check.
   * \return True if the directory exists, else false.
   */
  bool modPathExists(int mod_id) const;
  /*!
   * \brief Checks if the directory containing the given mod exists, if not logs an error.
   * \param mod_id If of the mod to check.
   * \return True if the directory exists, else false.
   */
  bool checkModPathExistsAndMaybeLogError(int mod_id) const;
  /*!
   * \brief Removes a legacy file that is no longer needed and may cause issues.
   * \param directory Directory from which to remove the file.
   */
  void removeManagedDirFile(const std::filesystem::path& directory) const;
  void setLoadorder(Json::Value entry, std::shared_ptr<TreeItem<DeployerEntry>> current);
};
