/*!
 * \file bg3deployer.h
 * \brief Header for the Bg3Deployer class
 */

#pragma once

#include "bg3pakfile.h"
#include "plugindeployer.h"
#include <map>


/*!
 * \brief Autonomous deployer which manages the modsettings.lsx file for Baldurs Gate 3.
 */
class Bg3Deployer : public PluginDeployer
{
public:
  /*!
   * \brief Loads mods.
   * \param source_path Path to the directory containing installed mods.
   * \param dest_path Path to the directory containing modsettings.lsx.
   * \param name A custom name for this instance.
   */
  Bg3Deployer(const std::filesystem::path& source_path,
              const std::filesystem::path& dest_path,
              const std::string& name);

  /*!
   * \brief If no backup exists: Backs up the current plugin file, then reloads all plugins.
   * \param progress_node Used to inform about the current progress.
   */
  virtual void unDeploy(std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Generates a vector of names for every plugin.
   * \return The name vector.
   */
  virtual std::vector<std::string> getModNames() const override;
  /*!
   * \brief Setter for the active profile. Changes the currently active plugin files
   * to the ones saved in the new profile.
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
  virtual std::unordered_set<int> getModConflicts(
    int mod_id,
    std::optional<ProgressNode*> progress_node = {}) override;

protected:
  /*! \brief Name of the mod settings file. */
  static constexpr std::string BG3_PLUGINS_FILE_NAME = "modsettings.lsx";
  /*! \brief Mod fixer is a popular mod which does not contain plugins. */
  static inline const std::set<std::string> NON_PLUGIN_ARCHIVES = { "ModFixer.pak" };
  /*! \brief Maps plugin UUIDs to the pak file containing them. */
  std::map<std::string, std::filesystem::path> uuid_map_;
  /*! \brief Maps pak file paths to the object containing that files plugin data. */
  std::map<std::filesystem::path, Bg3PakFile> pak_files_;

  /*! \brief Wrapper for \ref updatePluginsPrivate. */
  virtual void updatePlugins() override;
  /*! \brief Wrapper for \ref saveSettingsPrivate. */
  virtual void saveSettings() const override;
  /*! \brief Wrapper for \ref loadSettingsPrivate. */
  virtual void loadSettings() override;
  /*! \brief Resets profiles to 1 and active profile to 0. */
  virtual void resetSettings() override;
  /*! \brief Ensures plugins_ uuid_map_ and pak_files_ are coherent. */
  void cleanState();
  /*! \brief Wrapper for \ref writePluginsPrivate. */
  virtual void writePlugins() const override;
  /*! \brief Reads current plugins from modsettings.lsx and writes them to the plugins file. */
  bool initPluginFile();
  /*! \brief Updates the source mod map. */
  virtual void updateSourceMods() override;
  /*! \brief Tags are currently not supported by this type. */
  virtual void updatePluginTags() override;

private:
  /*! \brief Reads all pak files in the source directory and extracts all plugins. */
  void updatePluginsPrivate();
  /*! \brief Saves pak file and profile data to disk. */
  void saveSettingsPrivate() const;
  /*! \brief Reads pak file and profile data from disk. */
  void loadSettingsPrivate();
  /*! \brief Writes plugins to modsettings.lsx. */
  void writePluginsPrivate() const;
};
