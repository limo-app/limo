/*!
 * \file compressionerror.h
 * \brief Contains the CompressionError class
 */
#pragma once


/*!
 * \brief Exception used for errors during archive extraction.
 */
#include <stdexcept>
class CompressionError : public std::runtime_error
{
public:
  /*!
   * \brief Constructor accepts an error message.
   * \param message Exception message.
   */
  CompressionError(const char* message) : std::runtime_error(message){};
  /*!
   * \brief Returns the message of this exception.
   * \return The message.
   */
  const char* what() const throw() { return std::runtime_error::what(); };
};
