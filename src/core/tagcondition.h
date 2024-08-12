/*!
 * \file tagcondition.h
 * \brief Contains the TagCondition struct.
 */

#pragma once

#include <string>


/*!
 * \brief Contains data relevant to describing a single condition used for the application
 * of auto tags. This is used to construct a TagConditionNode.
 */
struct TagCondition
{
  /*! \brief Represents what should be compared to the search string. */
  enum class Type
  {
    /*! \brief Match against relative path, including file name. */
    path,
    /*! \brief Match against file name only. */
    file_name
  };

  /*! \brief If true: Matches only if condition is NOT met. */
  bool invert;
  /*! \brief Describes against what the search string should be matched. */
  Type condition_type;
  /*! \brief If true: Use regex matching, else use case insensitive matching with wildcards. */
  bool use_regex;
  /*! \brief This string will be matched against a given path. */
  std::string search_string;
};
