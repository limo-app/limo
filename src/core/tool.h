/*!
 * \file tool.h
 * \brief Header for the Tool class.
 */

#pragma once

#include <filesystem>
#include <json/json.h>
#include <map>


/*!
 * \brief Represents a third party tool to be run from within Limo.
 */
class Tool
{
public:
  /*! \brief Describes how the tool is to be run. */
  enum Runtime
  {
    /*! \brief Tool is to be run directly. */
    native,
    /*! \brief Tool is to be run through wine. */
    wine,
    /*! \brief Tool is to be run through proton by calling protontricks. */
    protontricks,
    /*! \brief Tool is a steam app. */
    steam
  };

  /*! \brief Default constructor */
  Tool() = default;
  /*!
   * \brief Constructs a tool that runs the given command directly.
   * \param name Name of the tool.
   * \param icon_path Path to the tool's icon.
   * \param command Command used to run the tool.
   */
  Tool(const std::string& name, const std::filesystem::path& icon_path, const std::string& command);
  /*!
   * \brief Constructs a tool using the native runtime.
   * \param name Name of the tool.
   * \param icon_path Path to the tool's icon.
   * \param executable_path Path to the tool's executable.
   * \param working_directory Working directory in which to run the command.
   * \param environment_variables Maps environment variables to their values.
   * \param arguments Arguments to be passed to the executable.
   */
  Tool(const std::string& name,
       const std::filesystem::path& icon_path,
       const std::filesystem::path& executable_path,
       const std::filesystem::path& working_directory,
       const std::map<std::string, std::string>& environment_variables,
       const std::string& arguments);
  /*!
   * \brief Constructs a tool using the wine runtime.
   * \param name Name of the tool.
   * \param icon_path Path to the tool's icon.
   * \param executable_path Path to the tool's executable.
   * \param prefix_path Path to the wine prefix_path.
   * \param working_directory Working directory in which to run the command.
   * \param environment_variables Maps environment variables to their values.
   * \param arguments Arguments to be passed to the executable.
   */
  Tool(const std::string& name,
       const std::filesystem::path& icon_path,
       const std::filesystem::path& executable_path,
       const std::filesystem::path& prefix_path,
       const std::filesystem::path& working_directory,
       const std::map<std::string, std::string>& environment_variables,
       const std::string& arguments);
  /*!
   * \brief Constructs a tool using the protontricks runtime.
   * \param name Name of the tool.
   * \param icon_path Path to the tool's icon.
   * \param executable_path Path to the tool's executable.
   * \param use_flatpak_protontricks Whether to use flatpak protontricks.
   * \param steam_app_id ID of the steam app containing the proton prefix.
   * \param working_directory Working directory in which to run the command.
   * \param environment_variables Maps environment variables to their values.
   * \param arguments Arguments to be passed to the executable.
   * \param protontricks_arguments Arguments to be passed to protontricks.
   */
  Tool(const std::string& name,
       const std::filesystem::path& icon_path,
       const std::filesystem::path& executable_path,
       bool use_flatpak_protontricks,
       int steam_app_id,
       const std::filesystem::path& working_directory,
       const std::map<std::string, std::string>& environment_variables,
       const std::string& arguments,
       const std::string& protontricks_arguments);
  /*!
   * \brief Constructs a tool using the steam runtime.
   * \param name Name of the tool.
   * \param icon_path Path to the tool's icon.
   * \param steam_app_id ID of the steam app to run.
   * \param use_flatpak_steam If true: Use the flatpak version of steam.
   */
  Tool(const std::string& name,
       const std::filesystem::path& icon_path,
       int steam_app_id,
       bool use_flatpak_steam);
  /*!
   * \brief Constructs a new Tool from data contained in the given JSON object.
   * \param json_object Source JSON object.
   */
  Tool(const Json::Value& json_object);

  /*!
   * \brief Constructs the command used to run this tool and returns it.
   * \param is_flatpak If true: The tool is to be run from within a flatpak sandbox.
   * \return The command.
   */
  std::string getCommand(bool is_flatpak) const;
  /*!
   * \brief Serializes this object to JSON.
   * \return The resulting JSON object.
   */
  Json::Value toJson() const;
  /*!
   * \brief Return this tool's name.
   * \return The name.
   */
  std::string getName() const;
  /*!
   * \brief Returns the path to an icon representing the tool.
   * \return The path.
   */
  std::filesystem::path getIconPath() const;
  /*!
   * \brief Returns the path to the executable of the tool.
   * \return The path.
   */
  std::filesystem::path getExecutablePath() const;
  /*!
   * \brief Returns the runtime used to run the tool.
   * \return The runtime.
   */
  Runtime getRuntime() const;
  /*!
   * \brief Returns true if flatpak version of protontricks or steam is used.
   * \return The status.
   */
  bool usesFlatpakRuntime() const;
  /*!
   * \brief Returns the path to the wine prefix.
   * \return The path.
   */
  std::filesystem::path getPrefixPath() const;
  /*!
   * \brief Returns the ID of the steam app containing the proton prefix.
   * \return The ID.
   */
  int getSteamAppId() const;
  /*!
   * \brief Returns the working directory in which to run the command.
   * \return The path.
   */
  std::filesystem::path getWorkingDirectory() const;
  /*!
   * \brief Returns a map containing environment variables and their values.
   * \return The map.
   */
  std::map<std::string, std::string> const getEnvironmentVariables() const;
  /*!
   * \brief Returns the arguments to be passed to the executable.
   * \return The arguments.
   */
  std::string getArguments() const;
  /*!
   * \brief Returns the arguments to be passed to protontricks.
   * \return The arguments.
   */
  std::string getProtontricksArguments() const;
  /*!
   * \brief Returns the overwrite command.
   * If this is not empty: Ignore all other settings and run this command directly.
   * \return The overwrite command.
   */
  std::string getCommandOverwrite() const;

private:
  /*! \brief Name of the tool. */
  std::string name_;
  /*! \brief Path to an icon representing the tool. */
  std::filesystem::path icon_path_;
  /*! \brief Path to the executable of the tool. */
  std::filesystem::path executable_path_;
  /*! \brief Runtime used to run the tool. */
  Runtime runtime_;
  /*! \brief If runtime is proton or steam: Whether to use the flatpak version. */
  bool use_flatpak_runtime_;
  /*! \brief If runtime is wine: Path to the wine prefix. */
  std::filesystem::path prefix_path_;
  /*!
   *  \brief If runtime is proton: ID of the steam app containing the proton prefix.
   *  If runtime is Steam: ID of the steam app to run.
   */
  int steam_app_id_;
  /*! \brief Working directory in which to run the command. */
  std::filesystem::path working_directory_;
  /*! \brief Maps environment variables to their values. */
  std::map<std::string, std::string> environment_variables_;
  /*! \brief Arguments to be passed to the executable. */
  std::string arguments_;
  /*! \brief Arguments to be passed to protontricks. */
  std::string protontricks_arguments_;
  /*! \brief If not empty: Ignore all other settings and run this command directly. */
  std::string command_overwrite_ = "";

  /*!
   * \brief Appends the given environment variables to the given command.
   * \param command Command to which to append to variables.
   * \param environment_variables Maps environment variables to their values
   * \param is_flatpak If true: Command is run from within a flatpak sandbox.
   */
  void appendEnvironmentVariables(std::string& command,
                                  const std::map<std::string, std::string>& environment_variables,
                                  bool is_flatpak) const;
  /*!
   * \brief Encloses the given string in quotes, if it is not already enclosed.
   * \param string String to be enclosed.
   * \return The enclosed string.
   */
  std::string encloseInQuotes(const std::string& string) const;
};
