/*!
 * \file plugindeployer.h
 * \brief Header for the PluginDeployer class
 */

#pragma once

#include "deployer.h"
#include <regex>

/*!
 * \brief Base class for autonomous deployers that collects all files which match a given critereon,
 * called plugins, in the source directory and adds them to a file in the target directory.
 */
class PluginDeployer : public Deployer
{
public:
  /*!
   * \brief Constructor
   * \param source_path Directory containing the plugin files.
   * \param dest_path Directory containing the file(s) into which plugin names are to be written.
   * \param name Custom name for this deployer instance.
   */
  PluginDeployer(const std::filesystem::path& source_path,
                 const std::filesystem::path& dest_path,
                 const std::string& name);

  /*!
   * \brief Reloads all deployed plugins.
   * \param progress_node Used to inform about the current progress of deployment.
   * \return Since this is an autonomous deployer, the returned map is always empty.
   */
  virtual std::map<int, unsigned long> deploy(
    std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Reloads all deployed plugins.
   * \param loadorder Ignored.
   * \param progress_node Used to inform about the current progress of deployment.
   * \return Since this is an autonomous deployer, the returned map is always empty.
   */
  virtual std::map<int, unsigned long> deploy(
    const std::vector<int>& loadorder,
    std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Moves a mod from one position in the load order to another. Saves changes to disk.
   * \param from_index Index of mod to be moved.
   * \param to_index Destination index.
   */
  virtual void swapChild(int from_index, int to_index) override;
  /*!
   * \brief Enables or disables the given mod in the load order. Saves changes to disk.
   * \param mod_id Mod to be edited.
   * \param status The new status.
   */
  virtual void setModStatus(int mod_id, bool status) override;
  /*!
   * \brief Conflict groups are not supported by this type.
   * \return All plugins in the non conflicting group.
   */
  virtual std::vector<std::vector<int>> getConflictGroups() const override;
  /*!
   * \brief Generates a vector of names for every plugin.
   * \return The name vector.
   */
  virtual std::vector<std::string> getModNames() const override;
  /*!
   * \brief Adds a new profile and optionally copies it's load order from an existing profile.
   * Profiles are stored in the target directory.
   * \param source The profile to be copied. A value of -1 indicates no copy.
   */
  virtual void addProfile(int source = -1) override;
  /*!
   * \brief Removes a profile.
   * \param profile The profile to be removed.
   */
  virtual void removeProfile(int profile) override;
  /*!
   * \brief Setter for the active profile. Changes the currently active plugin files
   * to the ones saved in the new profile.
   * \param profile The new profile.
   */
  virtual void setProfile(int profile) override;
  /*!
   * \brief Not supported by this type.
   * \param newConflict_groups Ignored.
   */
  virtual void setConflictGroups(const std::vector<std::vector<int>>& newConflict_groups) override;
  /*!
   * \brief Returns the number of plugins on the load order.
   * \return The number of plugins.
   */
  virtual int getNumMods() override;
  /*!
   * \brief Getter for the current plugin load order.
   * \return The load order.
   */
  virtual std::shared_ptr<TreeItem<DeployerEntry>> getLoadorder() override;
  /*!
   * \brief Does nothing since this deployer manages its own mods.
   * \param mod_id Ignored.
   * \param enabled Ignored.
   * \param update_conflicts Ignored.
   * \return False.
   */
  virtual bool addMod(int mod_id, bool enabled = true, bool update_conflicts = true) override;
  /*!
   * \brief Not supported by this type.
   * \param mod_id Ignored.
   * \return False.
   */
  virtual bool removeMod(int mod_id) override;
  /*!
   * \brief Since this deployer uses its own internal mod ids, this function always
   * returns false.
   * \param mod_id Ignores
   * \return False.
   */
  virtual bool hasMod(int mod_id) override;
  /*!
   * \brief Does nothing since this deployer manages its own mods.
   * \param old_id Ignored.
   * \param new_id Ignored
   * \return False.
   */
  virtual bool swapMod(int old_id, int new_id) override;
  /*!
   * \brief Not supported.
   * \param mod_id Ignored.
   * \param show_disabled Ignored.
   * \param progress_node Set to 100%.
   * \return An empty vector.
   */
  virtual std::vector<ConflictInfo> getFileConflicts(
    int mod_id,
    bool show_disabled = false,
    std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Not supported by this type.
   * \param mod_id The mod to be checked.
   * \param progress_node Used to inform about the current progress.
   * \return An empty set.
   */
  virtual std::unordered_set<int> getModConflicts(
    int mod_id,
    std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Not supported by this type.
   * \param progress_node Used to inform about the current progress.
   */
  virtual void sortModsByConflicts(std::optional<ProgressNode*> progress_node = {}) override;
  /*! \brief Deletes the config file and all profile files. */
  virtual void cleanup() override;
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
   * \brief Not supported by this Deployer type.
   * \param progress_node Ignored
   * \return An empty vector
   */
  virtual std::vector<std::pair<std::filesystem::path, int>> getExternallyModifiedFiles(
    std::optional<ProgressNode*> progress_node = {}) const override;
  /*!
   * \brief Not supported by this Deployer type.
   * \param changes_to_keep Ignored.
   */
  virtual void keepOrRevertFileModifications(const FileChangeChoices& changes_to_keep) override;
  /*!
   * \brief Updates the deployed files for one mod to match those in the mod's source directory.
   * This is not supported for this deployer type.
   * \param mod_id Ignored.
   * \param progress_node Ignored.
   */
  virtual void updateDeployedFilesForMod(
    int mod_id,
    std::optional<ProgressNode*> progress_node = {}) const override;
  /*! \brief Since this deployer type does not use normal deployment methods, this does nothing. */
  virtual void fixInvalidLinkDeployMode() override;
  /*!
   * \brief This deployer always uses copy deploy mode.
   * \param deploy_mode ignored.
   */
  virtual void setDeployMode(DeployMode deploy_mode) override;
  /*!
   * \brief Returns the order in which the deploy function of different
   *  deployers should be called.
   * \return The priority.
   */
  virtual int getDeployPriority() const override;
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
   * \brief Returns whether or not this deployer type uses mod ids as references to
   * source mods. This is usually done by autonomous deployers.
   * \return True
   */
  virtual bool idsAreSourceReferences() const override;
  /*!
   * \brief Returns a vector containing valid mod actions.
   * \return For every mod: IDs of every valid mod_action which is valid for that mod.
   */
  virtual std::vector<std::vector<int>> getValidModActions() const override;

protected:
  /*! \brief Appended to profile file names. */
  static constexpr std::string EXTENSION = ".lmmprof";
  /*! \brief File extension for plugins.txt and loadorder.txt backup files. */
  static constexpr std::string UNDEPLOY_BACKUP_EXTENSION = ".undeplbak";
  /*! \brief Name of the file containing settings. */
  std::string config_file_name_ = ".lmmconfig";
  /*! \brief Name of the file containing source mod ids for plugins. */
  std::string source_mods_file_name_ = ".lmm_mod_sources";

  /*! \brief Name of the file containing plugin activation status. */
  std::string plugin_file_name_ = "plugins.txt";
  /*! \brief Contains names of all plugins and their activation status. */
  std::vector<std::pair<std::string, bool>> plugins_;
  /*! \brief Current number of profiles. */
  int num_profiles_ = 0;
  /*! \brief For every plugin: Every tag associated with that plugin. */
  std::vector<std::vector<std::string>> tags_;
  /*! \brief Maps every plugin to a source mod, if that plugin was created by another deployer. */
  std::map<std::string, int> source_mods_;
  /*! \brief Regex used to match against files in the source directory. */
  std::regex plugin_regex_;
  /*! \brief Regex used to match against lines in the plugin file. */
  std::regex plugin_file_line_regex_;
  /*! \brief Name of the file containing loot tags. */
  std::string tags_file_name_ = ".plugin_tags";

  /*! \brief Updates current plugins to reflect plugins actually in the source directory. */
  virtual void updatePlugins();
  /*! \brief Load plugins from the plugins file. */
  virtual void loadPlugins();
  /*! \brief Writes current load order to plugins file. */
  virtual void writePlugins() const;
  /*!
   * \brief Saves number of profiles and active profile to the config file.
   */
  virtual void saveSettings() const;
  /*!
   * \brief Loads number of profiles and active profile from the config file.
   */
  virtual void loadSettings();
  /*! \brief Resets all settings to default values. */
  virtual void resetSettings();
  /*!
   *  \brief Updates the plugin tags for every currently loaded plugin.
   *  Must be implemented in derived classes.
   */
  virtual void updatePluginTags() = 0;
  /*! \brief Writes the current tags_ to disk. */
  virtual void writePluginTags() const;
  /*! \brief If plugin file backups exist, restore it and override the current file. */
  virtual void restoreUndeployBackupIfExists();
  /*! \brief Updates the source mod map with files created by another deployer. */
  virtual void updateSourceMods();
  /*! \brief Writes the source mods to disk. */
  virtual void writeSourceMods() const;
  /*! \brief Reads the source mods from disk. */
  virtual void readSourceMods();
  /*!
   * \brief Finds the directory serving as a target directory for the deployer which manages the
   * given target path.
   * \param target Target path to check.
   * \return The deployers target directory or an empty optional if no directory was found.
   */
  std::optional<std::filesystem::path> getRootOfTargetDirectory(std::filesystem::path target) const;
  /*!
   * \brief Converts the given file name to a hidden file by prepending a ".", if necessary.
   * \param name File name to hide.
   * \return The hidden file.
   */
  std::string hideFile(const std::string& name);
};
