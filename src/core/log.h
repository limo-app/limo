/*!
 * \file log.h
 * \brief Header for the Log namespace
 */
#pragma once

#include <filesystem>
#include <functional>
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
inline std::vector<std::function<void(std::string, LogLevel)>> log_printers;
/*! \brief Directory to which log files should be written. Empty string means no log files. */
inline std::filesystem::path log_dir = "";
/*! \brief Path to a log file. If this is != "" and exists, the log will be appended to that file. */
inline std::filesystem::path log_file_path = "";
/*! \brief Numberof log files to keep. One file is written per init() call. */
inline int num_log_files = 10;

/*!
 * \brief init Initializes the logger by setting the current log_file_path and renaming or deleting
 * old log files if needed.
 * \param log_dir_path Path to the logging directory.
 */
void init(std::filesystem::path log_dir_path = "");
/*!
 * \brief Prints the current time and date followed by a debug message.
 * \param message Message to be printed.
 * \param target_printer Log printer to use for output.
 */
void debug(const std::string& message, int target_printer = 0);
/*!
 * \brief Prints the current time and date followed by an info message.
 * \param message Message to be printed.
 * \param target_printer Log printer to use for output.
 */
void info(const std::string& message, int target_printer = 0);
/*!
 * \brief Prints the current time and date followed by a warning message.
 * \param message Message to be printed.
 * \param target_printer Log printer to use for output.
 */
void warning(const std::string& message, int target_printer = 0);
/*!
 * \brief Prints the current time and date followed by an error message.
 * \param message Message to be printed.
 * \param target_printer Log printer to use for output.
 */
void error(const std::string& message, int target_printer = 0);
/*!
 * \brief Calls the appropriate logging function for the given log level with the given message.
 * \param level Log level for the message.
 * \param message Message to be printed.
 * \param target_printer Log printer to use for output.
 */
void log(LogLevel level, const std::string& message, int target_printer = 0);
}
