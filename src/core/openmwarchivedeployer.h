/*!
 * \file openmwarchivedeployer.h
 * \brief Header for the OpenMwArchiveDeployer class
 */

#pragma once

#include "plugindeployer.h"


/*!
 * \brief Autonomous deployer which handles archive files for OpenMW.
 */
class OpenMwArchiveDeployer : public PluginDeployer
{
public:
  /*!
   * \brief Loads plugins.
   * \param source_path Path to the directory containing installed plugins.
   * \param dest_path Path to the directory containing openmw.cfg.
   * \param name A custom name for this instance.
   */
  OpenMwArchiveDeployer(const std::filesystem::path& source_path,
                        const std::filesystem::path& dest_path,
                        const std::string& name);

  /*!
   * \brief If no backup exists: Backs up current plugin file, then reloads all plugins.
   * \param progress_node Used to inform about the current progress.
   */
  virtual void unDeploy(std::optional<ProgressNode*> progress_node = {}) override;
  /*!
   * \brief Returns whether or not this deployer type supports sorting mods.
   * \return False.
   */
  virtual bool supportsSorting() const override;
  /*!
   * \brief Returns whether or not this deployer type supports showing mod conflicts.
   * \return False.
   */
  virtual bool supportsModConflicts() const override;

protected:
  /*! \brief Name of the OpenMW config file. */
  static constexpr std::string OPEN_MW_CONFIG_FILE_NAME = "openmw.cfg";

  /*! \brief Writes current load order to openmw config file. */
  virtual void writePlugins() const override;
  /*! \brief Tags are not supported by this type. */
  virtual void updatePluginTags() override;
  /*!
   *  \brief Initializes the plugin file, if it does not exist.
   *  \return A bool indicating if the plugin file was created.
   */
  bool initPluginFile();
};
