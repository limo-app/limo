/*!
 * \file log.h
 * \brief Header for the Log namespace
 */
#pragma once

#include <functional>
#include <iostream>
#include <string>


/*!
 * \brief Contains functions for logging.
 */
namespace Log
{
/*! \brief Represents the importance of a log message. */
enum LogLevel
{
  LOG_ERROR = 0,
  LOG_WARNING = 1,
  LOG_INFO = 2,
  LOG_DEBUG = 3
};

/*!
 * \brief Current log level. Messages with a log level less important than
 * this will be ignored.
 */
inline LogLevel log_level = LOG_INFO;
/*! \brief Callback function used to output log messages. */
inline std::function<void(std::string, LogLevel)> log_printer = [](std::string, LogLevel) {};


/*!
 * \brief Prints the current time and date followed by a debug message.
 * \param message Message to be printed.
 */
void debug(const std::string& message);
/*!
 * \brief Prints the current time and date followed by an info message.
 * \param message Message to be printed.
 */
void info(const std::string& message);
/*!
 * \brief Prints the current time and date followed by a warning message.
 * \param message Message to be printed.
 */
void warning(const std::string& message);
/*!
 * \brief Prints the current time and date followed by an error message.
 * \param message Message to be printed.
 */
void error(const std::string& message);
/*!
 * \brief Calls the appropriate logging function for the given log level with the given message.
 * \param level Log level for the message.
 * \param message Message to be printed.
 */
void log(LogLevel level, const std::string& message);
}
