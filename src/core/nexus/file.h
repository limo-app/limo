/*!
 * \file file.h
 * \brief Header for the nexus::File class.
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
 * \brief Contains data for a file on NexusMods.
 */
class File
{
public:
  /*!
   * \brief Constructor. Initializes all members from the given http response body generated
   * through an API request.
   * \param http_body The http response body.
   */
  File(const std::string& http_body);
  /*!
   * \brief Constructor. Initializes all members from the given http response body in json form
   * generated through an API request.
   * \param http_body The http response body in json form.
   */
  File(const Json::Value& json_body);
  /*! \brief Default constructor. */
  File() = default;

  /*! \brief The file id. */
  long id_0;
  /*! \brief The id of the domain containing mod to which the file belongs. */
  long id_1;
  /*! \brief Purpose unknown. */
  long uid;
  /*! \brief The file id. */
  long file_id;
  /*! \brief The name of the actual file on disk. */
  std::string name;
  /*! \brief The files version. */
  std::string version;
  /*! \brief Id of the category to which the file belongs. */
  long category_id;
  /*! \brief Name of the category to which the file belongs, e.g. MAIN. */
  std::string category_name;
  /*! \brief Purpose unknown. */
  bool is_primary;
  /*! \brief Size of the file in KibiBytes. */
  long size;
  /*! \brief The files display name- */
  std::string file_name;
  /*! \brief Timestamp for when the file was uploaded to NexusMods. */
  std::time_t uploaded_time;
  /*! \brief Mod version to which the file belongs. */
  std::string mod_version;
  /*! \brief Optional: The URL of a virus scanning website (like virustotal.com) for this file. */
  std::string external_virus_scan_url;
  /*! \brief The description if the file. */
  std::string description;
  /*! \brief Size of the file in KibiBytes. */
  long size_kb;
  /*! \brief Size of the file in Bytes. */
  long size_in_bytes;
  /*! \brief The changelog if the file. */
  std::string changelog_html;
  /*! \brief A URL of a NexusMods site showing a preview of the files contents. */
  std::string content_preview_link;

private:
  /*!
   * \brief Initializes all members from the given http response body in json form
   * generated through an API request.
   * \param http_body The http response body in json form.
   */
  void init(const Json::Value& json_body);
};
}
