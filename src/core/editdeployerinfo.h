/*!
 * \file editdeployerinfo.h
 * \brief Contains the EditDeployerInfo struct.
 */

#pragma once

#include "deployer.h"
#include <string>


/*!
 * \brief Stores data needed to either create a new or edit an existing
 * \ref Deployer "deployer".
 */
struct EditDeployerInfo
{
  /*! \brief Type of the deployer. */
  std::string type;
  /*! \brief Name of the deployer */
  std::string name;
  /*! \brief This is where the deployer will deploy to. */
  std::string target_dir;
  /*! \brief Determines how files will be deployed to the target directory. */
  Deployer::DeployMode deploy_mode;
  /*! \brief The deployers mod source directory. Only used by autonomous deployers. */
  std::string source_dir = "";
  /*!
   * \brief Used by ReverseDeployers: If true: Store files on a per profile basis.
   * Else: All profiles use the same files.
   */
  bool separate_profile_dirs = false;
  /*!
   * \brief Used by ReverseDeployers: If true: Add all files in target_dir not managed by another
   * deployer to the ignore list.
   */
  bool update_ignore_list = false;
};
