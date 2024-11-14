/*!
 * \file conflictinfo.h
 * \brief Contains the ConflictInfo struct.
 */

#pragma once

#include <string>
#include <vector>


/*!
 * \brief Stores information about a file conflict.
 */
struct ConflictInfo
{
  /*! \brief Name of the conflicting file. */
  std::string file;
  /*! \brief Id of the conflicts winning mod. */
  std::vector<int> mod_ids;
  /*! \brief Name of the conflicts winning mod. */
  std::vector<std::string> mod_names;
};
