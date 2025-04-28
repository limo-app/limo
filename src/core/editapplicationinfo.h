/*!
 * \file editapplicationinfo.h
 * \brief Contains the EditApplicationInfo struct.
 */

#pragma once

#include "editdeployerinfo.h"
#include <json/json.h>
#include <string>
#include <vector>


/*!
 * \brief Stores data needed to either create a new or edit an existing
 * \ref ModdedApplication "application".
 */
struct EditApplicationInfo
{
  /*! \brief New name of the application. */
  std::string name;
  /*! \brief Path to the staging directory. */
  std::string staging_dir;
  /*! \brief Command used to run the application. */
  std::string command;
  /*! \brief When creating a new application, this contains data needed to add initial deployers. */
  std::vector<EditDeployerInfo> deployers{};
  /*! \brief When creating a new application, this contains data needed to add initial auto tags. */
  std::vector<Json::Value> auto_tags{};
  /*!
   *  \brief When editing an application, this indicates whether to move the existing
   *  staging directory to the new path specified in staging_dir.
   */
  bool move_staging_dir = false;
  /*! \brief Path to the applications icon. */
  std::string icon_path;
  /*! \brief Version of the app. This is used for FOMOD conditions. */
  std::string app_version;
  /*! \brief Steam app id of the added application. */
  long steam_app_id;
};
