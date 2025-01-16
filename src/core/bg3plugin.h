/*!
 * \file bg3plugin.h
 * \brief Header for the Bg3Plugin class.
 */

#pragma once

#include <pugixml.hpp>
#include <set>
#include <string>
#include <vector>


/*!
 * \brief Parses and represents data from a meta.lsx file for a Baldurs Gate 3 plugin.
 */
class Bg3Plugin
{
public:
  /*!
   * \brief Initializes this object from the given xml string.
   * \param xml_string Source xml string.
   */
  Bg3Plugin(const std::string& xml_string);

  /*!
   * \brief Getter for the plugin UUID.
   * \return The UUID.
   */
  std::string getUuid() const;
  /*!
   * \brief Getter for the plugin version.
   * \return The version.
   */
  std::string getVersion() const;
  /*!
   * \brief Getter for the plugin directory.
   * \return The directory.
   */
  std::string getDirectory() const;
  /*!
   * \brief Getter for the plugin name.
   * \return The name.
   */
  std::string getName() const;
  /*!
   * \brief Getter for the plugin description.
   * \return The description.
   */
  std::string getDescription() const;
  /*!
   * \brief Getter for the plugin's dependencies.
   * \return for every dependency: A pair of UUID and name.
   */
  std::vector<std::pair<std::string, std::string>> getDependencies() const;
  /*!
   * \brief Checks if this plugin depends on the plugin with the given UUID.
   * \param uuid UUID to check.
   * \return True if dependency exists.
   */
  bool hasDependency(const std::string& uuid);
  /*!
   * \brief Compares this plugin's dependencies with the given plugin UUIDs.
   * \param plugin_uuids UUIDs to check.
   * \return For every dependency of this not in the given plugin UUIDs:
   * A pair of UUID and name of that dependency.
   */
  std::vector<std::pair<std::string, std::string>> getMissingDependencies(
    const std::set<std::string>& plugin_uuids);
  /*!
   * \brief Getter for the plugins xml representation.
   * \return The xml string.
   */
  std::string getXmlString() const;
  /*!
   * \brief Constructs an xml string for use in the Mods section of the modsettings.lsx file.
   * \return The xml string.
   */
  std::string toXmlPluginString() const;
  /*!
   * \brief Constructs an xml string for use in the ModOrder section of the modsettings.lsx file.
   * \return The xml string.
   */
  std::string toXmlLoadorderString() const;
  /*!
   * \brief Adds this plugin to the given Mods xml node in modsettings.lsx.
   * \param root Xml node to which to add this plugin.
   */
  void addToXmlModsNode(pugi::xml_node& root) const;
  /*!
   * \brief Adds this plugin to the given ModOrder xml node in modsettings.lsx.
   * \param root Xml node to which to add this plugin.
   */
  void addToXmlOrderNode(pugi::xml_node& root) const;
  /*!
   * \brief Checks if the given xml string contains a valid plugin that is not the GustavDev plugin.
   * \param xml_string Xml string to check.
   * \return True if the xml string contains a valid plugin.
   */
  static bool isValidPlugin(const std::string& xml_string);

  /*! \brief UUID of the GustavDev plugin. */
  static constexpr char BG3_VANILLA_MOD_UUID[] = "28ac9ce2-2aba-8cda-b3b5-6e922f71b6b8";

private:
  /*! \brief Xml representation of this plugin. */
  std::string xml_string_;
  /*! \brief UUID of this plugin. */
  std::string uuid_;
  /*! \brief Name of this plugin. */
  std::string version_;
  /*! \brief Subdirectory of this plugin. */
  std::string directory_;
  /*! \brief Name of this plugin. */
  std::string name_;
  /*! \brief Description of this plugin. */
  std::string description_;
  /*! \brief For every plugin dependency: A pair of its UUID and name. */
  std::vector<std::pair<std::string, std::string>> dependencies_;
};
