/*!
 * \file conflictinfo.h
 * \brief Contains the ConflictInfo struct.
 */

#pragma once

#include <string>


/*!
 * \brief Stores information about a file conflict.
 */
struct ConflictInfo
{
  /*! \brief Name of the conflicting file. */
  std::string file;
  /*! \brief Id of the conflicts winning mod. */
  int mod_id;
  /*! \brief Name of the conflicts winning mod. */
  std::string mod_name;
  /*!
   * \brief Constructor. Simply initializes members.
   * \param file Name of the conflicting file.
   * \param mod_id Id of the conflicts winning mod.
   * \param mod_name Name of the conflicts winning mod.
   */
  ConflictInfo(std::string file, int mod_id, std::string mod_name) :
    file(std::move(file)), mod_id(mod_id), mod_name(std::move(mod_name))
  {}
};
