/*!
 * \file editmanualtagaction.h
 * \brief Header for the EditManualTagAction class.
 */

#pragma once

#include <string>


/*!
 * \brief Contains data relevent for the action of editing a manual tag.
 */
class EditManualTagAction
{
public:
  /*! \brief Represents the type of action performed. */
  enum class ActionType
  {
    /*! \brief Add a new tag. */
    add,
    /*! \brief Remove an existing tag. */
    remove,
    /*! \brief Rename a tag. */
    rename
  };

  /*!
   * \brief Constructor.
   * \param name Name of the tag to be edited.
   * \param type Type of editing action to be performed.
   * \param new_name Contains the tags new name, if action is of type Rename.
   */
  EditManualTagAction(const std::string& name, ActionType type, const std::string& new_name = "");


  /*!
   * \brief Getter for the target tags name.
   * \return The name.
   */
  std::string getName() const;
  /*!
   * \brief Getter for the new name.
   * \return The new name.
   */
  std::string getNewName() const;
  /*!
   * \brief Getter for the ActionType to be performed.
   * \return The ActionType.
   */
  ActionType getType() const;

private:
  /*! \brief The target tags name. */
  std::string name_;
  /*! \brief The target tags new name, if ActionType == rename. */
  std::string new_name_;
  /*! \brief The type of action to be performed. */
  ActionType type_;
};
