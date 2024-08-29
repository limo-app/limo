#include "log.h"
#include <chrono>
#include <fstream>
#include <iomanip>

namespace sfs = std::filesystem;

inline constexpr std::string default_log_file_name = "limo_log";

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

void writeLog(const std::string& message, Log::LogLevel log_level)
{
  if(log_level >= Log::log_level)
    Log::log_printer(message, log_level);

  if(Log::log_file_path.empty())
    return;

  try
  {
    std::ofstream fstream(Log::log_file_path, std::ios::app);
    if(!fstream.is_open())
      return;
    fstream << message << "\n";
    fstream.flush();
  }
  catch(...)
  {
    Log::debug("Failed to write to log file!");
  }
}

namespace Log
{
void error(const std::string& message)
{
  writeLog(getTimestamp(Log::LOG_ERROR) + " [Error]: " + message, LOG_ERROR);
}

void warning(const std::string& message)
{
  writeLog(getTimestamp(Log::LOG_WARNING) + " [Warning]: " + message, LOG_WARNING);
}

void info(const std::string& message)
{
  writeLog(getTimestamp(Log::LOG_INFO) + " [Info]: " + message, LOG_INFO);
}

void debug(const std::string& message)
{
  writeLog(getTimestamp(Log::LOG_DEBUG) + " [Debug]: " + message, LOG_DEBUG);
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

void init(sfs::path log_dir_path)
{
  if(log_dir_path.empty())
    return;

  debug("Initializing config path to: " + log_dir_path.string());

  try
  {
    if(!sfs::exists(log_dir_path))
      sfs::create_directories(log_dir_path);

    const sfs::path max_file =
      log_dir_path / (default_log_file_name + "-" + std::to_string(num_log_files - 1));
    if(sfs::exists(max_file))
      sfs::remove(max_file);

    for(int i = num_log_files - 2; i >= 0; i--)
    {
      const sfs::path cur_file = log_dir_path / (default_log_file_name + "-" + std::to_string(i));
      const sfs::path prev_file =
        log_dir_path / (default_log_file_name + "-" + std::to_string(i + 1));
      if(sfs::exists(cur_file))
      {
        sfs::rename(cur_file, prev_file);
      }
    }

    log_file_path = log_dir_path / default_log_file_name;
    if(sfs::exists(log_file_path))
      sfs::rename(log_file_path, log_dir_path / (default_log_file_name + "-0"));
  }
  catch(...)
  {
    debug("Failed to initialize log directory!");
    return;
  }
}

}
