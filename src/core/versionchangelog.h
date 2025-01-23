/*!
 * \file versionchangelog.h
 * \brief Header for the VersionChangeLog class
 */

#pragma once

#include <json/json.h>
#include "changelogentry.h"


/*!
 * \brief Contains all changes made in a single version of Limo.
 */
class VersionChangelog
{
public:
  /*! \brief Default constructor. */
  VersionChangelog() = default;
  /*!
   * \brief Deserializes the changelog for a version from the given JSON object.
   * \param json JSON object containing the changelog.
   */
  VersionChangelog(const Json::Value& json);

  /*!
   * \brief Getter for the version string.
   * \return The version string.
   */
  std::string getVersion() const;
  /*!
   * \brief Getter for the publishing date of the version.
   * \return The date.
   */
  std::time_t getDate() const;
  /*!
   * \brief Getter for the version title.
   * \return The title.
   */
  std::string getTitle() const;
  /*!
   * \brief Getter for the changes made in this version.
   * \return The changes.
   */
  const std::vector<ChangelogEntry>& getChanges() const;
  /*!
   * \brief Constructs a string from the version and the date timestamp.
   * \return The string.
   */
  std::string versionAndDateString() const;
  /*!
   * \brief Compares publishing time of this with other.
   * \param other Changelog to compare to.
   * \return True if date of this is earlier than date of other.
   */
  bool operator < (const VersionChangelog& other) const;

private:
  /*! \brief Version number of this changelog. */
  std::string version_;
  /*! \brief Publishing date for this version. */
  std::time_t date_;
  /*! \brief Title for this version. */
  std::string title_;
  /*! \brief Contains all changes made in this version. */
  std::vector<ChangelogEntry> changes_;
};
