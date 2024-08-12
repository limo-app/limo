#include "log.h"
#include <chrono>
#include <iomanip>


std::string getTimestamp(Log::LogLevel log_level)
{
  const auto now = std::chrono::system_clock::now();
  auto cur_time = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&cur_time), "%F %T");
  if(log_level == Log::LOG_DEBUG)
    ss << "."
       << std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count() %
            1000;
  return ss.str();
}

namespace Log
{
void error(const std::string& message)
{
  if(log_level >= LOG_ERROR)
    log_printer(getTimestamp(Log::LOG_ERROR) + " [Error]: " + message, LOG_ERROR);
}

void warning(const std::string& message)
{
  if(log_level >= LOG_WARNING)
    log_printer(getTimestamp(Log::LOG_WARNING) + " [Warning]: " + message, LOG_WARNING);
}

void info(const std::string& message)
{
  if(log_level >= LOG_INFO)
    log_printer(getTimestamp(Log::LOG_INFO) + " [Info]: " + message, LOG_INFO);
}

void debug(const std::string& message)
{
  if(log_level >= LOG_DEBUG)
    log_printer(getTimestamp(Log::LOG_DEBUG) + " [Debug]: " + message, LOG_DEBUG);
}

void log(LogLevel level, const std::string& message)
{
  switch(level)
  {
    case LOG_DEBUG:
      debug(message);
      break;
    case LOG_INFO:
      info(message);
      break;
    case LOG_WARNING:
      warning(message);
      break;
    case LOG_ERROR:
      error(message);
      break;
    default:
      break;
  }
}
}
