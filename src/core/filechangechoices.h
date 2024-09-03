/*!
 * \file filechangechoices.h
 * \brief Header for the FileChangeChoices struct.
 */

#pragma once

#include <filesystem>
#include <vector>


/*!
 * \brief Contains data regarding which external file changes to keep.
 */
struct FileChangeChoices
{
  /*! \brief Contains paths to externally modified files, relative to the deployer's target directory. */
  std::vector<std::filesystem::path> paths{};
  /*! \brief For every modified file in \ref paths: The id of the mod containing that file. */
  std::vector<int> mod_ids{};
  /*!
   *  \brief For every modified file in \ref paths: A bool indicating if that change should be kept.
   *  true -> keep change; false -> reject change
   */
  std::vector<bool> changes_to_keep{};
};
