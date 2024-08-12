/*!
 * \file parseerror.h
 * \brief Contains the ParseError class.
 */

#pragma once

#include <stdexcept>


/*!
 * \brief Exception indicating an error while parsing a JSON file.
 */
class ParseError : public std::runtime_error
{
public:
  /*!
   * \brief Constructor.
   * \param message Message for the exception.
   */
  ParseError(const char* message) : std::runtime_error(message) {}
  /*!
   * \brief Constructor.
   * \param message Message for the exception.
   */
  ParseError(const std::string& message) : std::runtime_error(message) {}
};
