/*!
 * \file wildcardmatching.h
 * \brief Contains functions for wildcard matching strings.
 */

#pragma once

#include <string>
#include <vector>

/*!
 * \brief Checks if the given string matches the given expression string.
 * Uses * as a wildcard.
 * \param target String to compare to.
 * \param expression Wildcard expression string.
 * \return True if both match.
 */
bool wildcardMatch(const std::string& target, const std::string& expression);
/*!
 * \brief Splits the given string into substrings seperated by the * wildcard.
 * \param input String to split.
 * \return All substrings without the * wildcard.
 */
std::vector<std::string> splitString(const std::string& input);
