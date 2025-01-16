/*!
 * \file bg3pakfile.h
 * \brief Header for the Bg3PakFile class.
 */

#pragma once

#include "bg3plugin.h"
#include <filesystem>
#include <json/json.h>


/*!
 * \brief Parses and represents plugin data contained in a .pak file used in Baldurs Gate 3.
 */
class Bg3PakFile
{
public:
  /*! \brief Default constructor. */
  Bg3PakFile() = default;
  /*!
   * \brief Reads all plugins from the given .pak file.
   * \param source_file Source .pak file.
   */
  Bg3PakFile(const std::filesystem::path& source_file, const std::filesystem::path& prefix);
  /*!
   * \brief Deserializes data from the given json object.
   * \param json_value source json object.
   */
  Bg3PakFile(const Json::Value& json_value, const std::filesystem::path& prefix);

  /*!
   * \brief Returns a vector of plugins contained in this pak file.
   * \return The vector.
   */
  const std::vector<Bg3Plugin>& getPlugins() const;
  /*!
   * \brief Serializes this object.
   * \return Json object containing serialized data.
   */
  Json::Value toJson() const;
  /*!
   * \brief Returns the path to this object's source file.
   * \return The path.
   */
  std::filesystem::path getSourceFile() const;
  /*!
   * \brief Checks if this file's timestamp matches the modification time on disk.
   * \return True of the times match.
   */
  bool timestampsMatch();
  /*!
   * \brief Returns the name of the plugin matching the given UUID.
   * \param uuid UUID to check.
   * \return The name, or an empty string of the UUID could not be matched.
   */
  std::string getPluginName(const std::string& uuid) const;
  /*!
   * \brief Checks whether a plugin with the given UUID exists.
   * \param uuid UUID to check.
   * \return True if a plugin exists.
   */
  bool hasPlugin(const std::string& uuid) const;
  /*!
   * \brief Checks whether the given plugin conflicts with the given plugin in the given file.
   * \param plugin_uuid Plugin UUID in this file to check.
   * \param other_file Other file containing other_plugin.
   * \param other_plugin_uuid Plugin UUID in other_file.
   * \return True if a conflict exists.
   */
  bool pluginConflictsWith(const std::string& plugin_uuid,
                           const Bg3PakFile& other_file,
                           const std::string& other_plugin_uuid);
  /*!
   * \brief Checks whether the any files conflcits with the given pak archive.
   * \param other Archive to check.
   * \return True if a conflict exists.
   */
  bool conflictsWith(const Bg3PakFile& other);

    private :
    /*! \brief Contains all plugins in the source file. */
    std::vector<Bg3Plugin> plugins_;
  /*! \brief Path to the source file. */
  std::filesystem::path source_file_;
  /*! \brief Time at which the source file was modified. */
  std::time_t modified_time_;
  /*! \brief Contains paths to files in the source file. */
  std::vector<std::filesystem::path> file_list_;
  /*! \brief Prefix for the source file path. */
  std::filesystem::path source_path_prefix_;

  /*! \brief Initializes this object from the source file. */
  void init();
  /*! \brief Reads the file modification time from the given file. */
  std::time_t getTimestamp(const std::filesystem::path& file);
};
