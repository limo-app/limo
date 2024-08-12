/*!
 * \file mod.h
 * \brief Header for the nexus::Mod class.
 */

#pragma once

#include <chrono>
#include <json/json.h>
#include <string>


/*!
 * \brief The nexus namespace contains structs and functions needed for accessing the NexusMods API.
 */
namespace nexus
{
/*!
 * \brief Contains data for a mod on NexusMods.
 */
class Mod
{
public:
  /*! \brief Default constructor. */
  Mod() = default;
  /*!
   * \brief Constructor. Initializes all members from the given http response body generated
   * through an API request.
   * \param http_body The http response body.
   */
  Mod(const std::string& http_body);
  /*!
   * \brief Constructor. Initializes all members from the given http response body in json form
   * generated through an API request.
   * \param http_body The http response body in json form.
   */
  Mod(const Json::Value& json_body);

  /*! \brief Name of the mod. */
  std::string name;
  /*! \brief A summary of the mods contents. */
  std::string summary;
  /*! \brief The long form description of the mod. */
  std::string description;
  /*! \brief URL of the main image representing the mod. */
  std::string picture_url;
  /*! \brief Total number of downloads for the mod. */
  long mod_downloads;
  /*! \brief Total number of unique downloads for the mod. */
  long mod_unique_downloads;
  /*! \brief Purpose unknown. */
  long uid;
  /*! \brief NexusMods mod id. */
  long mod_id;
  /*! \brief Id of the NexusMods domain containing the mod. */
  long game_id;
  /*! \brief If true: Mod can be rated. */
  bool allow_rating;
  /*! \brief Name of the NexusMods domain containing the mod. */
  std::string domain_name;
  /*! \brief Id of the NexusMods mod category for the mod. */
  long category_id;
  /*! \brief Most recent mod version. */
  std::string version;
  /*! \brief Number of endorsements of the mod. */
  long endorsement_count;
  /*! \brief Timestamp for when the mod was first uploaded to NexusMods. */
  std::time_t created_time;
  /*! \brief Timestamp for when the mod was first last updated. */
  std::time_t updated_time;
  /*! \brief Name of the mods author. */
  std::string author;
  /*! \brief Name of the mod uploader. */
  std::string uploaded_by;
  /*! \brief URL to the NexusMods account which uploaded the mod. */
  std::string uploaded_users_profile_url;
  /*! \brief True if the mod contains adult content. */
  bool contains_adult_content;
  /*! \brief The current status of the mod, e.g. Published. */
  std::string status;
  /*! \brief True if the mod is available........ */
  bool available;
  /*! \brief User id of the uploader. */
  long user_member_id;
  /*! \brief A group id for the uploader. */
  long user_member_group_id;
  /*! \brief Name of the uploader. */
  std::string user_name;
  /*! \brief Endorsement status of the mod for the account used to fetch the mod data. */
  std::string endorsement_status;

private:
  /*!
   * \brief Initializes all members from the given http response body in json form
   * generated through an API request.
   * \param http_body The http response body in json form.
   */
  void init(const Json::Value& json_body);
};
}
