#include "tool.h"
#include <ranges>

namespace sfs = std::filesystem;


Tool::Tool(const std::string& name, const sfs::path& icon_path, const std::string& command) :
  name_(name), icon_path_(icon_path), runtime_(native), command_overwrite_(command)
{}

Tool::Tool(const std::string& name,
           const sfs::path& icon_path,
           const sfs::path& executable_path,
           const sfs::path& working_directory,
           const std::map<std::string, std::string>& environment_variables,
           const std::string& arguments) :
  name_(name), icon_path_(icon_path), executable_path_(executable_path), runtime_(native),
  working_directory_(working_directory), environment_variables_(environment_variables),
  arguments_(arguments)
{}

Tool::Tool(const std::string& name,
           const sfs::path& icon_path,
           const sfs::path& executable_path,
           const sfs::path& prefix_path,
           const sfs::path& working_directory,
           const std::map<std::string, std::string>& environment_variables,
           const std::string& arguments) :
  name_(name), icon_path_(icon_path), executable_path_(executable_path), runtime_(wine),
  prefix_path_(prefix_path), working_directory_(working_directory),
  environment_variables_(environment_variables), arguments_(arguments)
{}

Tool::Tool(const std::string& name,
           const sfs::path& icon_path,
           const sfs::path& executable_path,
           bool use_flatpak_protontricks,
           int steam_app_id,
           const sfs::path& working_directory,
           const std::map<std::string, std::string>& environment_variables,
           const std::string& arguments,
           const std::string& protontricks_arguments) :
  name_(name), icon_path_(icon_path), executable_path_(executable_path), runtime_(protontricks),
  use_flatpak_protontricks_(use_flatpak_protontricks), steam_app_id_(steam_app_id),
  working_directory_(working_directory), environment_variables_(environment_variables),
  arguments_(arguments), protontricks_arguments_(protontricks_arguments)
{}

Tool::Tool(const Json::Value& json_object)
{
  name_ = json_object["name"].asString();
  icon_path_ = json_object["icon_path"].asString();
  executable_path_ = json_object["executable_path"].asString();
  runtime_ = static_cast<Runtime>(json_object["runtime"].asInt());
  use_flatpak_protontricks_ = json_object["use_flatpak_protontricks"].asBool();
  prefix_path_ = json_object["prefix_path"].asString();
  steam_app_id_ = json_object["steam_app_id"].asInt();
  working_directory_ = json_object["working_directory"].asString();
  for(int i = 0; i < json_object["environment_variables"].size(); i++)
  {
    environment_variables_[json_object["environment_variables"][i]["variable"].asString()] =
      json_object["environment_variables"][i]["value"].asString();
  }
  arguments_ = json_object["arguments"].asString();
  protontricks_arguments_ = json_object["protontricks_arguments"].asString();
  command_overwrite_ = json_object["command_overwrite"].asString();
}

std::string Tool::getCommand(bool is_flatpak) const
{
  if(!command_overwrite_.empty())
    return is_flatpak ? "flatpak-spawn --host " + encloseInQuotes(command_overwrite_)
                      : encloseInQuotes(command_overwrite_);

  std::string command;
  if(is_flatpak)
    command = "flatpak-spawn --host";

  if(!command.empty())
    command += " ";
  if(!working_directory_.empty())
  {
    if(is_flatpak)
      command += "--directory=" + encloseInQuotes(working_directory_.string());
    else
      command += "cd " + encloseInQuotes(working_directory_.string()) + ";";
  }

  appendEnvironmentVariables(command, environment_variables_, is_flatpak);
  if(runtime_ == wine && !prefix_path_.empty())
    appendEnvironmentVariables(command, { { "WINEPREFIX", prefix_path_.string() } }, is_flatpak);

  if(!command.empty() && runtime_ != native)
    command += " ";
  if(runtime_ == wine)
    command += "wine";
  else if(runtime_ == protontricks)
  {
    if(use_flatpak_protontricks_)
      command += "flatpak run --command=protontricks-launch com.github.Matoking.protontricks ";
    else
      command += "protontricks-launch ";
    command += "--appid " + std::to_string(steam_app_id_);
    if(!protontricks_arguments_.empty())
      command += " " + protontricks_arguments_;
  }

  if(!command.empty())
    command += " ";
  command += encloseInQuotes(executable_path_);

  if(!arguments_.empty())
    command += " " + arguments_;

  return command;
}

Json::Value Tool::toJson() const
{
  Json::Value json_object;
  json_object["name"] = name_;
  json_object["icon_path"] = icon_path_.string();
  json_object["executable_path"] = executable_path_.string();
  json_object["runtime"] = static_cast<int>(runtime_);
  json_object["use_flatpak_protontricks"] = use_flatpak_protontricks_;
  json_object["prefix_path"] = prefix_path_.string();
  json_object["steam_app_id"] = steam_app_id_;
  json_object["working_directory"] = working_directory_.string();
  for(const auto& [i, pair] : std::views::enumerate(environment_variables_))
  {
    const auto& [variable, value] = pair;
    json_object["environment_variables"][(int)i]["variable"] = variable;
    json_object["environment_variables"][(int)i]["value"] = value;
  }
  json_object["arguments"] = arguments_;
  json_object["protontricks_arguments"] = protontricks_arguments_;
  json_object["command_overwrite"] = command_overwrite_;
  return json_object;
}

std::string Tool::getName() const
{
  return name_;
}

sfs::path Tool::getIconPath() const
{
  return icon_path_;
}

sfs::path Tool::getExecutablePath() const
{
  return executable_path_;
}

Tool::Runtime Tool::getRuntime() const
{
  return runtime_;
}

bool Tool::usesFlatpakProtontricks() const
{
  return use_flatpak_protontricks_;
}

sfs::path Tool::getPrefixPath() const
{
  return prefix_path_;
}

int Tool::getSteamAppId() const
{
  return steam_app_id_;
}

sfs::path Tool::getWorkingDirectory() const
{
  return working_directory_;
}

const std::map<std::string, std::string> Tool::getEnvironmentVariables() const
{
  return environment_variables_;
}

std::string Tool::getArguments() const
{
  return arguments_;
}

std::string Tool::getProtontricksArguments() const
{
  return protontricks_arguments_;
}

std::string Tool::getCommandOverwrite() const
{
  return command_overwrite_;
}

void Tool::appendEnvironmentVariables(
  std::string& command,
  const std::map<std::string, std::string>& environment_variables,
  bool is_flatpak) const
{
  for(const auto& [variable, value] : environment_variables)
  {
    if(!command.empty())
      command += " ";
    if(is_flatpak)
      command += "--env=";
    command += variable + "=" + encloseInQuotes(value);
  }
}

std::string Tool::encloseInQuotes(const std::string& string) const
{
  if(string.starts_with('"') && string.ends_with('"'))
    return string;

  return '"' + string + '"';
}
