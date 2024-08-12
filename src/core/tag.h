/*!
 * \file tag.h
 * \brief Header for the Tag class.
 */

#pragma once

#include <json/json.h>
#include <string>
#include <vector>


/*!
 * \brief Abstract base class for a tag assigned to a set of mods.
 */
class Tag
{
public:
  /*!
   * \brief Getter for the tags name.
   * \return The name.
   */
  std::string getName() const;
  /*!
   * \brief Setter for the tags name.
   * \param name The new name.
   */
  void setName(const std::string& name);
  /*!
   * \brief Returns all mods to which this tag has been added.
   * \return A vector of mods ids.
   */
  std::vector<int> getMods() const;
  /*!
   * \brief Returns the number of mods to which this tag has been added.
   * \return The number of mods.
   */
  int getNumMods() const;
  /*!
   * \brief Checks if this tag has been added to the given mod.
   * \param mod_id Mod to be checked.
   * \return True if the given mod has this tag.
   */
  bool hasMod(int mod_id) const;
  /*!
   * \brief Serializes this tag to a json object.
   * This function must be implemented by derived classes.
   * \return The json object.
   */
  virtual Json::Value toJson() const = 0;

protected:
  /*! \brief Name of this tag. */
  std::string name_;
  /*! \brief Contains ids of all mods to which this tag has been added. */
  std::vector<int> mods_{};
};
