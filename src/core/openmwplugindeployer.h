/*!
 * \file openmwplugindeployer.h
 * \brief Header for the OpenMwPluginDeployer class
 */

#pragma once

#include "lootdeployer.h"


/*!
 * \brief Autonomous deployer which handles plugin files for OpenMW using LOOT.
 */
class OpenMwPluginDeployer : public LootDeployer
{
public:
  /*!
   * \brief Loads plugins.
   * \param source_path Path to the directory containing installed plugins.
   * \param dest_path Path to the directory containing openmw.cfg.
   * \param name A custom name for this instance.
   * \param init_tags If true: Initializes plugin tags. Disable this for testing purposes
   * with invalid plugin files
   */
  OpenMwPluginDeployer(const std::filesystem::path& source_path,
                 const std::filesystem::path& dest_path,
                 const std::string& name,
                 bool init_tags = true);

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
   * \brief Setter for the active profile. Changes the currently active plugins file
   * to the one saved in the new profile.
   * \param profile The new profile.
   */
  virtual void setProfile(int profile) override;

private:
  /*! \brief Name of the OpenMW config file. */
  static constexpr std::string OPEN_MW_CONFIG_FILE_NAME = "openmw.cfg";

  /*! \brief Writes plugins to the OpenMW config file. */
  void writePlugins() const override;
  /*!
   *  \brief Initializes the plugin file, if it does not exist.
   *  \return A bool indicating if the plugin file was created.
   */
  bool initPluginFile();
};
