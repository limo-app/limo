#include "log.h"
#include <chrono>
#include <fstream>
#include <iomanip>

namespace sfs = std::filesystem;

inline constexpr std::string default_log_file_name = "limo_log";
inline constexpr std::string default_log_file_extension = ".txt";

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

void writeLog(const std::string& message, Log::LogLevel log_level, int target_printer = 0)
{
  if(Log::log_level >= log_level && Log::log_printers.size() > target_printer)
    Log::log_printers[target_printer](message, log_level);

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
    if(Log::log_printers.size() > target_printer)
      Log::log_printers[target_printer]("Failed to write to log file!", Log::LOG_DEBUG);
  }
}

std::string getOldLogFileName(int log_num)
{
  return default_log_file_name + "-" + std::to_string(log_num);
}

std::string getLogFileName(int log_num)
{
  return getOldLogFileName(log_num) + default_log_file_extension;
}

namespace Log
{
void error(const std::string& message, int target_printer)
{
  writeLog(getTimestamp(Log::LOG_ERROR) + " [Error]: " + message, LOG_ERROR, target_printer);
}

void warning(const std::string& message, int target_printer)
{
  writeLog(getTimestamp(Log::LOG_WARNING) + " [Warning]: " + message, LOG_WARNING, target_printer);
}

void info(const std::string& message, int target_printer)
{
  writeLog(getTimestamp(Log::LOG_INFO) + " [Info]: " + message, LOG_INFO, target_printer);
}

void debug(const std::string& message, int target_printer)
{
  writeLog(getTimestamp(Log::LOG_DEBUG) + " [Debug]: " + message, LOG_DEBUG, target_printer);
}

void log(LogLevel level, const std::string& message, int target_printer)
{
  switch(level)
  {
    case LOG_DEBUG:
      debug(message, target_printer);
      break;
    case LOG_INFO:
      info(message, target_printer);
      break;
    case LOG_WARNING:
      warning(message, target_printer);
      break;
    case LOG_ERROR:
      error(message, target_printer);
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

    // rename files created before the .txt extension was used
    if(sfs::exists(log_dir_path / default_log_file_name))
      sfs::rename(log_dir_path / default_log_file_name,
                  log_dir_path / (default_log_file_name + default_log_file_extension));
    for(int i = 0; i < num_log_files; i++)
    {
      if(sfs::exists(log_dir_path / getOldLogFileName(i)))
        sfs::rename(log_dir_path / getOldLogFileName(i), log_dir_path / getLogFileName(i));
    }

    // delete the oldest log, if file limit has been exceeded
    const sfs::path max_file = log_dir_path / getLogFileName(num_log_files - 1);
    sfs::remove(max_file);

    // rename existing files to make room for a new log file
    for(int i = num_log_files - 2; i >= 0; i--)
    {
      const sfs::path cur_file = log_dir_path / getLogFileName(i);
      const sfs::path prev_file = log_dir_path / getLogFileName(i + 1);
      if(sfs::exists(cur_file))
        sfs::rename(cur_file, prev_file);
    }

    log_file_path = log_dir_path / (default_log_file_name + default_log_file_extension);
    if(sfs::exists(log_file_path))
      sfs::rename(log_file_path, log_dir_path / getLogFileName(0));
  }
  catch(...)
  {
    debug("Failed to initialize log directory!");
    return;
  }
}

}
