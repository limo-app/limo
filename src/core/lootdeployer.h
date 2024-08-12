/*!
 * \file lootdeployer.h
 * \brief Header for the LootDeployer class
 */
#pragma once

#include "deployer.h"
#include "loot/api.h"
#include <json/json.h>


/*!
 * \brief Autonomous Deployer which handles plugins for Fallout 3, Fallout 4,
 * Fallout New Vegas, Fallout 4 VR, Starfield, Morrowind, Oblivion, Skyrim,
 * Skyrim SE and Skyrim VR.
 */
class LootDeployer : public Deployer
{
public:
  /*!
   * \brief Loads plugins and identifies the app type to be managed.
   * \param source_path Path to the directory containing installed plugins.
   * \param dest_path Path to the directory containing plugins.txt and loadorder.txt.
   * \param name A custom name for this instance.
   * \param init_tags If true: Initializes plugin tags. Disable this for testing purposes
   * with invalid plugin files
   */
  LootDeployer(const std::filesystem::path& source_path,
               const std::filesystem::path& dest_path,
               const std::string& name,
               bool init_tags = true);

  /*! \brief Maps game type to a URL pointing to the masterlist.yaml for that type. */
  static inline const std::map<loot::GameType, std::string> DEFAULT_LIST_URLS = {
    { loot::GameType::fo3,
      "https://raw.githubusercontent.com/loot/fallout3/master/masterlist.yaml" },
    { loot::GameType::fo4,
      "https://raw.githubusercontent.com/loot/fallout4/master/masterlist.yaml" },
    { loot::GameType::fo4vr,
      "https://raw.githubusercontent.com/loot/fallout4vr/master/masterlist.yaml" },
    { loot::GameType::fonv,
      "https://raw.githubusercontent.com/loot/falloutnv/master/masterlist.yaml" },
    { loot::GameType::starfield,
      "https://raw.githubusercontent.com/loot/starfield/master/masterlist.yaml" },
    { loot::GameType::tes3,
      "https://raw.githubusercontent.com/loot/morrowind/master/masterlist.yaml" },
    { loot::GameType::tes4,
      "https://raw.githubusercontent.com/loot/oblivion/master/masterlist.yaml" },
    { loot::GameType::tes5,
      "https://raw.githubusercontent.com/loot/skyrim/master/masterlist.yaml" },
    { loot::GameType::tes5se,
      "https://raw.githubusercontent.com/loot/skyrimse/master/masterlist.yaml" },
    { loot::GameType::tes5vr,
      "https://raw.githubusercontent.com/loot/skyrimvr/master/masterlist.yaml" }
  };

  static inline std::map<loot::GameType, std::string> LIST_URLS;

  /*!
   * \brief Reloads all deployed plugins. Does NOT save current load order to disk.
   * \param progress_node Used to inform about the current progress of deployment.
   * \return Since this is an autonomous deployer, the returned map is always empty.
   */
  std::map<int, unsigned long> deploy(std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Reloads all deployed plugins. Does NOT save current load order to disk.
   * \param loadorder Ignored.
   * \param progress_node Used to inform about the current progress of deployment.
   * \return Since this is an autonomous deployer, the returned map is always empty.
   */
  std::map<int, unsigned long> deploy(const std::vector<int>& loadorder,
                                      std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Moves a mod from one position in the load order to another. Saves changes to disk.
   * \param from_index Index of mod to be moved.
   * \param to_index Destination index.
   */
  void changeLoadorder(int from_index, int to_index) override;
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
   * \brief Generates a vector of names for every plugin.
   * \return The name vector.
   */
  std::vector<std::string> getModNames() const override;
  /*!
   * \brief Adds a new profile and optionally copies it's load order from an existing profile.
   * Profiles are stored in the target directory.
   * \param source The profile to be copied. A value of -1 indicates no copy.
   */
  void addProfile(int source = -1) override;
  /*!
   * \brief Removes a profile.
   * \param profile The profile to be removed.
   */
  void removeProfile(int profile) override;
  /*!
   * \brief Setter for the active profile. Changes the currently active loadorder.txt
   * and plugin.txt to the ones saved in the new profile.
   * \param profile The new profile.
   */
  void setProfile(int profile) override;
  /*!
   * \brief Does nothing.
   * \param newConflict_groups Ignored.
   */
  void setConflictGroups(const std::vector<std::vector<int>>& newConflict_groups) override;
  /*!
   * \brief Returns the number of plugins on the load order.
   * \return The number of plugins.
   */
  int getNumMods() const override;
  /*!
   * \brief Getter for the current plugin load order.
   * \return The load order.
   */
  std::vector<std::tuple<int, bool>> getLoadorder() const override;
  /*!
   * \brief Does nothing since this deployer manages its own mods.
   * \param mod_id Ignored.
   * \param enabled Ignored.
   * \param update_conflicts Ignored.
   * \return False.
   */
  bool addMod(int mod_id, bool enabled = true, bool update_conflicts = true) override;
  /*!
   * \brief Does nothing.
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
  bool hasMod(int mod_id) const override;
  /*!
   * \brief swapMod Does nothing since this deployer manages its own mods.
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
    std::optional<ProgressNode*> progress_node = {}) const override;
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
   * \brief Sorts the current load order using LOOT. Uses a masterlist.yaml appropriate
   * for the game managed by this deployer and optionally a userlist.yaml in the target
   * directory. Saves the new load order to disk after sorting.
   * \param progress_node Used to inform about the current progress.
   */
  void sortModsByConflicts(std::optional<ProgressNode*> progress_node = {}) override;
  /*! \brief Deletes the config file and all profile files. */
  void cleanup() override;
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

private:
  /*! \brief Name of the file containing plugin activation status. */
  static constexpr std::string PLUGIN_FILE_NAME = "plugins.txt";
  /*! \brief Name of the file containing plugin load order. */
  static constexpr std::string LOADORDER_FILE_NAME = "loadorder.txt";
  /*! \brief Appended to profile file names. */
  static constexpr std::string EXTENSION = ".lmmprof";
  /*! \brief Name of the file containing settings. */
  static constexpr std::string CONFIG_FILE_NAME = ".lmmconfig";
  /*! \brief Name of the file containing loot tags. */
  static constexpr std::string TAGS_FILE_NAME = ".loot_tags";
  /*! \brief Maps supported game type to a path to a file unique to that type. */
  static inline const std::map<loot::GameType, std::filesystem::path> TYPE_IDENTIFIERS = {
    { loot::GameType::fo3, "Fallout3.esm" },
    { loot::GameType::fo4, "Fallout4.esm" },
    { loot::GameType::fo4vr, "Fallout4_VR.esm" },
    { loot::GameType::fonv, "FalloutNV.esm" },
    { loot::GameType::starfield, "Starfield.esm" },
    { loot::GameType::tes3, "Morrowind.esm" },
    { loot::GameType::tes4, "Oblivion.esm" },
    { loot::GameType::tes5, std::filesystem::path("..") / "TESV.exe" },
    { loot::GameType::tes5se, std::filesystem::path("..") / "SkyrimSE.exe" },
    { loot::GameType::tes5vr, "SkyrimVR.esm" }
  };
  /*! \brief Name of a light plugin tag. */
  static constexpr std::string LIGHT_PLUGIN = "Light";
  /*! \brief Name of a master plugin tag. */
  static constexpr std::string MASTER_PLUGIN = "Master";
  /*! \brief Name of a standard plugin tag. */
  static constexpr std::string STANDARD_PLUGIN = "Standard";
  /*! \brief Contains names of all plugins and their activation status. */
  std::vector<std::pair<std::string, bool>> plugins_;
  /*! \brief Contains names of plugins which are in loadorder.txt but not in plugins.txt. */
  std::vector<std::string> prefix_plugins_;
  /*! \brief Current number of profiles. */
  int num_profiles_ = 0;
  /*! \brief Type of game to be managed. */
  loot::GameType app_type_;
  /*! \brief Timestamp representing the last time the masterlist.yaml was updated. */
  long list_download_time_ = 0;
  /*! \brief If true: Automatically download new master lists. */
  bool auto_update_lists_ = true;
  /*! \brief Current number of light plugins. */
  int num_light_plugins_ = 0;
  /*! \brief Current number of master plugins. */
  int num_master_plugins_ = 0;
  /*! \brief Current number of standard plugins. */
  int num_standard_plugins_ = 0;
  /*! \brief For every plugin: Every loot tag associated with that plugin. */
  std::vector<std::vector<std::string>> tags_;

  /*! \brief Updates current plugins to reflect plugins actually in the source directory. */
  void updatePlugins();
  /*! \brief Load plugins from plugins.txt and loadorder.txt. */
  void loadPlugins();
  /*! \brief Writes current load order to plugins.txt and loadorder.txt. */
  void writePlugins() const;
  /*!
   * \brief Saves number of profiles, active profile, list_download_time_ and
   * auto_update_lists_ to the config file.
   */
  void saveSettings() const;
  /*!
   * \brief Loads number of profiles, active profile, list_download_time_ and
   * auto_update_lists_ from the config file.
   */
  void loadSettings();
  /*! \brief Identifies the type of game in the source directory using signature files. */
  void updateAppType();
  /*! \brief Downloads a new masterlist.yaml, if the current one is older than a day. */
  void updateMasterList();
  /*! \brief Resets all settings to default values. */
  void resetSettings();
  /*! \brief Creates plugin.txt and loadorder.txt files if they do not exist. */
  void setupPluginFiles();
  /*! \brief Updates the loot plugin tags for every currently loaded plugin. */
  void updatePluginTags();
  /*! \brief Writes the current tags_ to disk. */
  void writePluginTags() const;
  /*! \brief Reads tags_ from disk. */
  void readPluginTags();
};
