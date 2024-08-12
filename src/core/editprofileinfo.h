/*!
 * \file editprofileinfo.h
 * \brief Contains the EditProfileInfo struct.
 */

#pragma once

#include <string>


/*!
 * \brief Stores data needed to either create a new or edit an existing
 * profile of a \ref ModdedApplication "application".
 */
struct EditProfileInfo
{
  /*! \brief The new name of the profile. */
  std::string name;
  /*! \brief The new app version of the profile. Used for FOMOD conditions. */
  std::string app_version;
  /*! \brief If a new profile is created and this is != -1: Copy all settings from source profile.
   */
  int source = -1;
};
