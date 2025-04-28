/*!
 * \file rootlevelcondition.h
 * \brief Header for the RootLevelCondition class.
 */

#pragma once

#include <QTreeWidgetItem>
#include <json/json.h>
#include <optional>
#include <string>


/*!
 * \brief Used to find the root level during mod installation using a regex or wildcard expression.
 */
class RootLevelCondition
{
public:
  /*! \brief Type of string matcher to use on file names. */
  enum MatcherType
  {
    /*! \brief Wildcard matcher. */
    simple,
    /*! \brief Regex matcher. */
    regex
  };

  /*! \brief Describes what file type the expression should be matched against. */
  enum TargetType
  {
    /*! \brief Any file. */
    any,
    /*! \brief Only non directories. */
    file,
    /*! \brief Only directories. */
    directory
  };

  /*!
   * \brief Simply initializes members.
   * \param matcher_type Type of file name matcher to use.
   * \param target_type Only files of this type will be used for matching.
   * \param case_invariant If true: Use case invariant matching.
   * \param stop_on_branch If true: Stop root level detection as soon as a directory contains more
   * than one entry.
   * \param expression Used for matching.
   * \param level_offset Offset to add to the detected root level.
   */
  RootLevelCondition(MatcherType matcher_type,
                     TargetType target_type,
                     bool case_invariant,
                     bool stop_on_branch,
                     const std::string& expression,
                     int level_offset);
  /*!
   * \brief Deserializes a RootLevelCondition from the given JSON object.
   * \param json Contains values for members.
   */
  RootLevelCondition(const Json::Value& json);

  /*!
   * \brief Recursively matches the given expression to every node in the given (sub)tree.
   *
   * WARNING: The root node itself is ignored for matching.
   *
   * \param root_node Root of the (sub)tree for which to match.
   * \param cur_level Current level in the tree.
   * \return If possible: The level at which the expression first matches a root_node's text.
   */
  std::optional<int> detectRootLevel(QTreeWidgetItem* root_node, int cur_level = 0) const;

private:
  /*! \brief JSON key name for the matcher type. */
  static inline constexpr char JSON_MATCHER_TYPE_KEY[] = "matcher_type";
  /*! \brief JSON key name for the target type. */
  static inline constexpr char JSON_TARGET_TYPE_KEY[] = "target_type";
  /*! \brief JSON key name for case invariance.. */
  static inline constexpr char JSON_CASE_INVARIANT_KEY[] = "case_invariant";
  /*! \brief JSON key name for the expression. */
  static inline constexpr char JSON_EXPRESSION_KEY[] = "expression";
  /*! \brief JSON key name for stop on branch. */
  static inline constexpr char JSON_STOP_KEY[] = "stop_on_branch";
  /*! \brief JSON key name the level offset. */
  static inline constexpr char JSON_OFFSET_KEY[] = "level_offset";
  /*! \brief JSON value for the wildcard matcher. */
  static inline constexpr char JSON_MATCHER_SIMPLE_VALUE[] = "simple";
  /*! \brief JSON value for the regex matcher. */
  static inline constexpr char JSON_MATCHER_REGEX_VALUE[] = "regex";
  /*! \brief JSON value for any file type. */
  static inline constexpr char JSON_TARGET_ANY_VALUE[] = "any";
  /*! \brief JSON value for non directory file type. */
  static inline constexpr char JSON_TARGET_FILE_VALUE[] = "file";
  /*! \brief JSON value for directory file type. */
  static inline constexpr char JSON_TARGET_DIRECTORY_VALUE[] = "directory";

  /*! \brief Type of file name matcher to use. */
  MatcherType matcher_type_;
  /*! \brief Only files of this type will be used for matching. */
  TargetType target_type_;
  /*! \brief If true: Use case invariant matching. */
  bool case_invariant_matching_;
  /*! \brief If true: Stop root level detection as soon as a directory contains more
   * than one entry. */
  bool stop_on_branch_;
  /*! \brief Used for matching. */
  std::string expression_;
  /*! \brief Offset to add to the detected root level. */
  int level_offset_;
};
