/*!
 * \file editapplicationinfo.h
 * \brief Contains the EditApplicationInfo struct.
 */

#pragma once

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
  /*!
   *  \brief When creating a new application, this contains names and target paths
   *  for initial deployers.
   */
  std::vector<std::pair<std::string, std::string>> deployers;
  /*!
   *  \brief When editing an application, this indicates whether to move the existing
   *  staging directory to the new path specified in staging_dir.
   */
  bool move_staging_dir = false;
  /*! \brief Path to the applications icon. */
  std::string icon_path;
  /*! \brief Version of the app. This is used for FOMOD conditions. */
  std::string app_version;
};
