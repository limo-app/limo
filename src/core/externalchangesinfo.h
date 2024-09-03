/*!
 * \file externalchangesinfo.h
 * \brief Contains the ExternalChangesInfo struct.
 */


#pragma once

#include <vector>
#include <string>
#include <filesystem>


/*!
 * \brief Contains data regarding externally modified files for one deployer.
 */
struct ExternalChangesInfo
{
  /*!
   * \brief For every modified file: Its path, relative to the deployer's target directory
   * and the id of the mod from which that file was deployed.
   */
  std::vector<std::pair<std::filesystem::path, int>> file_changes;
  /*! \brief Name of the deployer. */
  std::string deployer_name;
  /*! \brief Id of the deployer. */
  int deployer_id;
};
