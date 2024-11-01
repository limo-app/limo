#pragma once

#include "deployer.h"


class DeployerFactory
{
public:
  DeployerFactory() = delete;

  /*! \brief Performs no additional actions. */
  inline static const std::string SIMPLEDEPLOYER{ "Simple Deployer" };
  /*!
   *  \brief Uses case insensitive string matching when comparing
   *  mod file names with target file names.
   */
  inline static const std::string CASEMATCHINGDEPLOYER{ "Case Matching Deployer" };
  /*! \brief Manages Bethesda plugin files using LOOT. */
  inline static const std::string LOOTDEPLOYER{ "Loot Deployer" };
  /*! \brief Moves all files not from a mod out of the target directory and links them back in. */
  inline static const std::string REVERSEDEPLOYER{ "Reverse Deployer" };
  /*!
   * \brief Returns a vector of available deployer types.
   * \return The vector of deployer types.
   */
  /*! \brief Contains all available deployer types. */
  inline static const std::vector<std::string> DEPLOYER_TYPES{ CASEMATCHINGDEPLOYER,
                                                               SIMPLEDEPLOYER,
                                                               LOOTDEPLOYER,
                                                               REVERSEDEPLOYER };
  /*! \brief Maps deployer types to a description of what they do. */
  inline static const std::map<std::string, std::string> DEPLOYER_DESCRIPTIONS{
    { SIMPLEDEPLOYER,
      "Links/ copies all files from enabled mods in its loadorder into "
      "target directory. Backs up and restores existing files when needed." },
    { CASEMATCHINGDEPLOYER,
      "When the target directory contains a file with the same name "
      "but different case as a mods file name, renames the mods name to "
      "match the target file. Then deploys as normal." },
    { LOOTDEPLOYER,
      "Uses LOOT to manage plugins for games like Skyrim. Source path "
      "should point to the directory which plugins are installed into."
      "Target path should point to the directory containing plugins.txt "
      "and loadorder.txt" },
    { REVERSEDEPLOYER,
      "Moves all files not managed by another deployer out of the target directory and "
      "links them back in on deployment. Can be used to either track files created by"
      "other mods or to manage save files for different profiles." }
  };
  /*! \brief Maps deployer types to a bool indicating
   *  if the type refers to an autonomous deployer. */
  inline static const std::map<std::string, bool> AUTONOMOUS_DEPLOYERS{ { SIMPLEDEPLOYER, false },
                                                                        { CASEMATCHINGDEPLOYER,
                                                                          false },
                                                                        { LOOTDEPLOYER, true },
                                                                        { REVERSEDEPLOYER, true } };
  /*!
   * \brief Constructs a unique pointer to a new deployer of given type.
   * \param type Deployer type to be constructed.
   * \param source_path Path to directory containing mods installed using the Installer class.
   * \param dest_path Path to target directory for mod deployment.
   * \param name A custom name for this instance.
   * \param deploy_mode Determines how files are deployed to the target directory
   * \param separate_profile_dirs Used by ReverseDeployers: If true: Store files on a per profile basis.
   * Else: All profiles use the same files.
   * \param update_ignore_list Used by ReverseDeployers: If true: Add all files in target_dir
   * not managed by another deployer to the ignore list.
   * \return The constructed unique pointer.
   */
  static std::unique_ptr<Deployer> makeDeployer(
    const std::string& type,
    const std::filesystem::path& source_path,
    const std::filesystem::path& dest_path,
    const std::string& name,
    Deployer::DeployMode deploy_mode = Deployer::hard_link,
    bool separate_profile_dirs = false,
    bool update_ignore_list = false);
};
