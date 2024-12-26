/*!
 * \file lootdeployer.h
 * \brief Header for the LootDeployer class
 */
#pragma once

#include "deployer.h"
#include "loot/api.h"
#include "plugindeployer.h"
#include <json/json.h>


/*!
 * \brief Autonomous Deployer which handles plugins for Fallout 3, Fallout 4,
 * Fallout New Vegas, Fallout 4 VR, Starfield, Morrowind, Oblivion, Skyrim,
 * Skyrim SE and Skyrim VR.
 */
class LootDeployer : public PluginDeployer
{
public:
  /*!
   * \brief Loads plugins and identifies the app type to be managed.
   * \param source_path Path to the directory containing installed plugins.
   * \param dest_path Path to the directory containing plugins.txt and loadorder.txt.
   * \param name A custom name for this instance.
   * \param init_tags If true: Initializes plugin tags. Disable this for testing purposes
   * with invalid plugin files
   * \param perform_init If true: Perform initialization.
   */
  LootDeployer(const std::filesystem::path& source_path,
               const std::filesystem::path& dest_path,
               const std::string& name,
               bool init_tags = true,
               bool perform_init = true);

  /*! \brief Maps game type to a URL pointing to the masterlist.yaml for that type. */
  static inline const std::map<loot::GameType, std::string> DEFAULT_LIST_URLS = {
    { loot::GameType::fo3,
      "https://raw.githubusercontent.com/loot/fallout3/v0.21/masterlist.yaml" },
    { loot::GameType::fo4,
      "https://raw.githubusercontent.com/loot/fallout4/v0.21/masterlist.yaml" },
    { loot::GameType::fo4vr,
      "https://raw.githubusercontent.com/loot/fallout4vr/v0.21/masterlist.yaml" },
    { loot::GameType::fonv,
      "https://raw.githubusercontent.com/loot/falloutnv/v0.21/masterlist.yaml" },
    { loot::GameType::starfield,
      "https://raw.githubusercontent.com/loot/starfield/v0.21/masterlist.yaml" },
    { loot::GameType::tes3,
      "https://raw.githubusercontent.com/loot/morrowind/v0.21/masterlist.yaml" },
    { loot::GameType::tes4,
      "https://raw.githubusercontent.com/loot/oblivion/v0.21/masterlist.yaml" },
    { loot::GameType::tes5, "https://raw.githubusercontent.com/loot/skyrim/v0.21/masterlist.yaml" },
    { loot::GameType::tes5se,
      "https://raw.githubusercontent.com/loot/skyrimse/v0.21/masterlist.yaml" },
    { loot::GameType::tes5vr,
      "https://raw.githubusercontent.com/loot/skyrimvr/v0.21/masterlist.yaml" }
  };
  /*! \brief Has to be initialized with the URLs actually used for downloading masterlists. */
  static inline std::map<loot::GameType, std::string> LIST_URLS;
  /*! \brief Default URL used to download the masterlist prelude. */
  static inline const std::string DEFAULT_PRELUDE_URL =
    "https://raw.githubusercontent.com/loot/prelude/v0.21/prelude.yaml";
  /*! \brief URL actually used to download the prelude.yaml file. Has to be initialized. */
  static inline std::string PRELUDE_URL;

  /*!
   * \brief If no backup exists: Backs up current plugin file, then reloads all plugins.
   * \param progress_node Used to inform about the current progress.
   */
  virtual void unDeploy(std::optional<ProgressNode*> progress_node = {}) override;
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
   * \brief Setter for the active profile. Changes the currently active loadorder.txt
   * and plugin.txt to the ones saved in the new profile.
   * \param profile The new profile.
   */
  virtual void setProfile(int profile) override;
  /*!
   * \brief Checks for conflicts with other mods.
   * Two mods are conflicting if they share at least one record.
   * \param mod_id The mod to be checked.
   * \param progress_node Used to inform about the current progress.
   * \return A set of mod ids which conflict with the given mod.
   */
  virtual std::unordered_set<int> getModConflicts(int mod_id,
                                          std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Sorts the current load order using LOOT. Uses a masterlist.yaml appropriate
   * for the game managed by this deployer and optionally a userlist.yaml in the target
   * directory. Saves the new load order to disk after sorting.
   * \param progress_node Used to inform about the current progress.
   */
  virtual void sortModsByConflicts(std::optional<ProgressNode*> progress_node = {}) override;
  /*! \brief Deletes the config file and all profile files. */
  virtual void cleanup() override;
  /*!
   * \brief Returns all available auto tag names.
   * \return The tag names.
   */
  virtual std::map<std::string, int> getAutoTagMap() override;

protected:
  /*! \brief Name of the file containing plugin load order. */
  static constexpr std::string LOADORDER_FILE_NAME = "loadorder.txt";
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
  /*! \brief Maps loot game types to the default name used to store plugins. */
  static inline const std::map<loot::GameType, std::string> PLUGIN_FILE_NAMES = {
    { loot::GameType::fo3, "plugins.txt" },       { loot::GameType::fo4, "plugins.txt" },
    { loot::GameType::fo4vr, "plugins.txt" },     { loot::GameType::fonv, "plugins.txt" },
    { loot::GameType::starfield, "plugins.txt" }, { loot::GameType::tes3, "plugins.txt" },
    { loot::GameType::tes4, "Plugins.txt" },      { loot::GameType::tes5, "plugins.txt" },
    { loot::GameType::tes5se, "plugins.txt" },    { loot::GameType::tes5vr, "plugins.txt" }
  };

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

  /*! \brief Writes current load order to plugins.txt and loadorder.txt. */
  virtual void writePlugins() const override;
  /*!
   * \brief Saves number of profiles, active profile, list_download_time_ and
   * auto_update_lists_ to the config file.
   */
  virtual void saveSettings() const override;
  /*!
   * \brief Loads number of profiles, active profile, list_download_time_ and
   * auto_update_lists_ from the config file.
   */
  void loadSettings() override;
  /*! \brief Identifies the type of game in the source directory using signature files. */
  void updateAppType();
  /*! \brief Downloads a new masterlist.yaml, if the current one is older than a day. */
  virtual void updateMasterList();
  /*! \brief Resets all settings to default values. */
  virtual void resetSettings() override;
  /*! \brief Creates plugin.txt and loadorder.txt files if they do not exist. */
  void setupPluginFiles();
  /*! \brief Updates the loot plugin tags for every currently loaded plugin. */
  virtual void updatePluginTags() override;
  /*! \brief Reads tags_ from disk. */
  void readPluginTags();
  /*! \brief Downloads the file from the given URL and stores it at dest_path_/file_name. */
  virtual void downloadList(std::string url, const std::string& file_name);
  /*! \brief If loadorder and plugin file backups exist, restore them and override the current files. */
  virtual void restoreUndeployBackupIfExists() override;
};
