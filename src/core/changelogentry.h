/*!
 * \file changelogentry.h
 * \brief Header for the ChangeLogEntry class
 */

#pragma once

#include <json/json.h>


/*!
 * \brief Contains data for a single change in the changelog.
 */
class ChangelogEntry
{
public:
  /*! \brief Represents the type of change for a changelog entry. */
  enum ChangeType
  {
    new_feature = 0,
    change = 1,
    fix = 2,
    no_type = 100
  };

  /*! \brief Default constructor. */
  ChangelogEntry() = default;
  /*!
   * \brief Deserializes a changelog entry from the given JSON object.
   * \param json JSON object containing the changelog entry.
   */
  ChangelogEntry(const Json::Value& json);

  /*!
   * \brief Getter for the change type.
   * \return The type.
   */
  ChangeType getType() const;
  /*!
   * \brief Getter for the short description.
   * \return The description.
   */
  std::string getShortDescription() const;
  /*!
   * \brief Getter for the long description.
   * \return The description.
   */
  std::string getLongDescription() const;
  /*!
   * \brief Getter for the GitHub issue ID.
   * \return The issue ID.
   */
  int getIssue() const;
  /*!
   * \brief Getter for the GitHub pull request ID.
   * \return The pull request ID.
   */
  int getPullRequest() const;
  /*!
   * \brief Compares this and other by their type.
   * \param other ChangelogEntry to compare to.
   * \return True if type of this is less than type of other.
   */
  bool operator<(const ChangelogEntry& other) const;

private:
  /*! \brief Type of this change. */
  ChangeType type_;
  /*! \brief Short description of the change. */
  std::string short_description_;
  /*! \brief Optional: A more detailed description or an explanation. */
  std::string long_description_;
  /*! \brief GitHub issue addressed by this change, or -1 if not related to any issue. */
  int issue_ = -1;
  /*! \brief GitHub pull request responsible for this change or -1 if not related to a PR. */
  int pull_request_ = -1;
};
