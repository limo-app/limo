/*!
 * \file tagconditionnode.h
 * \brief Header for the TagConditionNode class.
 */

#pragma once

#include "tagcondition.h"
#include <filesystem>
#include <map>
#include <vector>


/*!
 * \brief Represents a node in a tree used to model a boolean expression for
 * evaluating if the files in a directory match a set of conditions.
 */
class TagConditionNode
{
public:
  /*! \brief Type of this node. */
  enum class Type
  {
    /*! \brief Node evaluates to true only if all children evaluate to true. */
    and_connector,
    /*! \brief Node evaluates to true if at least one child evaluates to true. */
    or_connector,
    /*! \brief Leaf node. Evaluates to true if a file name matches a pattern. */
    file_matcher,
    /*! \brief Leaf node. Evaluates to true if a file path matches a pattern. */
    path_matcher,
    /*! \brief Dummy node. Always evaluates to false. */
    empty
  };

  /*! \brief Constructs a node of type empty. */
  TagConditionNode();
  /*!
   * \brief Constructs a new node from the given boolean expression and conditions.
   * Recursively constructs children as needed. Node types are deduced from the expression.
   * \param expression Expression used to construct the tree.
   * \param conditions Conditions which serve as variables in the expression.
   */
  TagConditionNode(std::string expression, const std::vector<TagCondition>& conditions);

  /*!
   * \brief Checks if files in the given vector satisfy
   * the boolean expression modeled by this tree node.
   * \param files Contains pairs of path and file names for all files of a mod.
   * \return True if the directory satisfies the expression.
   */
  bool evaluate(const std::vector<std::pair<std::string, std::string>>& files) const;
  /*!
   * \brief Removes all outer parentheses that serve no semantic purpose in the given expression.
   * \param expression Expression to be modified.
   */
  static void removeEnclosingParentheses(std::string& expression);
  /*!
   * \brief Checks if the given string is a syntactically valid boolean expression.
   * \param exppression String to validate.
   * \param num_conditions Number of conditions available in the expression.
   * \return True if the expression is valid.
   */
  static bool expressionIsValid(std::string expression, int num_conditions);

private:
  /*! \brief The boolean expression modeled by this tree. */
  std::string expression_;
  /*! \brief If true: Invert the evaluation result. */
  bool invert_ = false;
  /*! \brief Child nodes of this node. */
  std::vector<TagConditionNode> children_;
  /*! \brief Type of this node. */
  Type type_;
  /*! \brief String used to comparisons in leaf nodes. */
  std::string condition_;
  /*! \brief Used to store substrings of the expression. Split by the * wildcard. */
  std::vector<std::string> condition_strings_;
  /*!
   *  \brief If this is a leaf: Represents the condition in the tree. Used to avoid
   *  evaluating conditions multiple times.
   */
  int condition_id_;
  /*!
   * \brief If true: Use regex to compare against the condition string.
   *  Else: Use a simple string matcher with * as a wildcard.
   */
  bool use_regex_;

  /*!
   * \brief Checks if files in the given vector satisfy
   * the boolean expression modeled by this tree node. This check is skipped if the given
   * results map contains this nodes id.
   * \param files Contains pairs of path and file names for all files of a mod.
   * \param results Contains results of previous evaluations.
   * \return True if the directory satisfies the expression.
   */
  bool evaluateOnce(const std::vector<std::pair<std::string, std::string>>& files,
                    std::map<int, bool>& results) const;
  /*!
   * \brief Checks if files in the given vector satisfy
   * the boolean expression modeled by this tree node. This check is skipped if the given
   * results map contains this nodes id.
   * Does not invert the result even is invert_ is true.
   * \param files Contains pairs of path and file names for all files of a mod.
   * \param results Contains results of previous evaluations.
   * \return True if the directory satisfies the expression.
   */
  bool evaluateWithoutInversion(const std::vector<std::pair<std::string, std::string>>& files,
                                std::map<int, bool>& results) const;
  /*!
   * \brief Checks if the given expression contains the given boolean operator.
   * Only checks the top level part of the expression.
   * \param expression Expression to check.
   * \param op Operator used for comparison.
   * \return True if expression contains operator.
   */
  bool containsOperator(const std::string& expression, const std::string& op) const;
  /*!
   * \brief Splits the given expression into tokens. Tokens are either condition ids,
   * boolean operators or a subexpression in parentheses.
   * \param expression Expression to split.
   * \return Contains pairs of index and length of tokens in the given expression.
   */
  std::vector<std::pair<int, int>> tokenize(const std::string& expression) const;
  /*!
   * \brief Removes all whitespaces in the given string.
   * \param expression Expression to modify.
   */
  void removeWhitespaces(std::string& expression) const;
  /*!
   * \brief Checks if the given string matches this nodes condition_ string.
   * Uses * as a wildcard.
   * \param target String to compare to.
   * \return True if both match.
   */
  bool wildcardMatch(const std::string& target) const;
  /*!
   * \brief Splits the given string into substrings seperated by the * wildcard.
   * \param input String to split.
   * \return All substrings without the * wildcard.
   */
  std::vector<std::string> splitString(const std::string& input) const;
  /*!
   * \brief Checks if the order of operators in the given boolean expression is valid.
   * \param expression Expression to check.
   * \return True if the order is valid.
   */
  static bool operatorOrderIsValid(std::string expression);
  /*!
   * \brief Removes all occurrences of substring from string.
   * \param string String from which to remove.
   * \param substring Substring to remove.
   */
  static void removeSubstring(std::string& string, std::string substring);
};
