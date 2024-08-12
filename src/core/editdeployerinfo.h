/*!
 * \file editdeployerinfo.h
 * \brief Contains the EditDeployerInfo struct.
 */

#pragma once

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
  /*! \brief If true: Copy mods to target directory, else: use hard links. */
  bool use_copy_deployment;
  /*! \brief The deployers mod source directory. Only used by autonomous deployers. */
  std::string source_dir = "";
};
