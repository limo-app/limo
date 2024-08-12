/*!
 * \file manualtag.h
 * \brief Header for the ManualTag class.
 */

#pragma once

#include "tag.h"
#include <json/json.h>
#include <string>
#include <vector>


/*!
 * \brief Tag which has to be manually added to mods.
 */
class ManualTag : public Tag
{
public:
  /*!
   * \brief Constructs a new tag with the given name.
   * \param name The tags name.
   */
  ManualTag(std::string name);
  /*!
   * \brief Deserializes a ManualTag from the given json object.
   * \param json Source json object.
   * \param json_path Path to the json object. Used is exception messaged.
   * \throws ParseError when the json object is invalid.
   */
  ManualTag(const Json::Value& json);

  /*!
   * \brief Adds this tag to the given mod.
   * \param mod_id Id if the mod to which this tag is to be added.
   */
  void addMod(int mod_id);
  /*!
   * \brief Removes this tag from the given mod.
   * \param mod_id Id if the mod from which this tag is to be removed.
   */
  void removeMod(int mod_id);
  /*!
   * \brief Removes this tag from all mods and adds it only to the given mods.
   * \param mods Mods to which this tag is to be added.
   */
  void setMods(const std::vector<int> mods);
  /*!
   * \brief Serializes this tag to a json object.
   * \return The json object.
   */
  Json::Value toJson() const;
  /*!
   * \brief Compares this tag by name to the given name.
   * \param name Name to compare to.
   * \return True if the names are identical.
   */
  bool operator==(const std::string& name) const;
  /*!
   * \brief Compares this tag by name to the given tag.
   * \param other Tag to compare to.
   * \return True if the names are identical.
   */
  bool operator==(const ManualTag& other) const;
};
