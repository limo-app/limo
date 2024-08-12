/*!
 * \file mod.h
 * \brief Contains the Mod struct.
 */

#pragma once

#include <filesystem>
#include <json/json.h>
#include <string>


/*!
 * \brief Stores information about an installed mod.
 */
struct Mod
{
  /*! \brief The mod's id. */
  int id;
  /*! \brief The mod's name. */
  std::string name;
  /*! \brief The mod's version. */
  std::string version;
  /*! \brief The mod's installation time. */
  std::time_t install_time;
  /*! \brief Path to the local archive or directory used to install this mod. */
  std::filesystem::path local_source;
  /*! \brief URL from where the mod was downloaded. */
  std::string remote_source;
  /*! \brief Timestamp for when the mod was updated at the remote source. */
  std::time_t remote_update_time;
  /*! \brief Total size of the installed mod on disk. */
  unsigned long size_on_disk;
  /*! \brief Timestamp for when the user requested to suppress current update notifications. */
  std::time_t suppress_update_time;

  /*!
   * \brief Constructor. Simply initializes members.
   * \param id The mod's id.
   * \param name The mod's name.
   * \param version The mod's version.
   * \param time The mod's installation time.
   * \param source_l Path to the local archive or directory used to install this mod.
   * \param source_r URL from where the mod was downloaded.
   * \param time_r Timestamp for when the mod was updated at the remote source.
   * \param size Total size of the installed mod on disk.
   * \param suppress_time Timestamp for when the user requested to suppress current update
   * notifications.
   */
  Mod(int id,
      const std::string& name,
      const std::string& version,
      const std::time_t& time,
      const std::filesystem::path& source_l,
      const std::string& source_r,
      const std::time_t& time_r,
      unsigned long size,
      const std::time_t& suppress_time);
  /*!
   * \brief Initializes all members from a JSON object.
   * \param json The source for member values.
   */
  Mod(const Json::Value& json);

  Json::Value toJson() const;
  /*!
   * \brief Compares to another mod by id.
   * \param other Mod to compare to.
   * \return True if both share the same id, else false.
   */
  bool operator==(const Mod& other) const;
  /*!
   * \brief Compares mods by their id.
   * \param Other mod for comparison.
   * \return True only if this.id < other.id
   */
  bool operator<(const Mod& other) const;
};
