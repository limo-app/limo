/*!
 * \file editautotagaction.h
 * \brief Header for the EditAutoTagAction class.
 */

#pragma once

#include "tagcondition.h"
#include <string>
#include <vector>


/*!
 * \brief Contains data relevent for the action of editing an auto tag.
 */
class EditAutoTagAction
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
    rename,
    /*! \brief Create a new evaluator. */
    change_evaluator
  };

  /*!
   * \brief Constructor for an add or remove action.
   * \param name Name of the tag to be added/ removed.
   * \param type Action type.
   */
  EditAutoTagAction(const std::string& name, ActionType type);
  /*!
   * \brief Constructor for a rename action.
   * \param name Name of the tag to be renamed.
   * \param new_name New name for the tag.
   */
  EditAutoTagAction(const std::string& name, const std::string& new_name);
  /*!
   * \brief Constructor for a change_evaluator action.
   * \param name Name of the tag the evaluator of which is to be updated.
   * \param expression New evaluator expression.
   * \param conditions New evaluator conditions.
   */
  EditAutoTagAction(const std::string& name,
                    const std::string& expression,
                    const std::vector<TagCondition>& conditions);


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
  /*!
   * \brief Getter for the expression of the updated evaluator.
   * \return The expression.
   */
  std::string getExpression() const;
  /*!
   * \brief Getter for the conditions of the updated evaluator.
   * \return The conditions.
   */
  std::vector<TagCondition> getConditions() const;

private:
  /*! \brief The target tags name. */
  std::string name_;
  /*! \brief The target tags new name, if ActionType == rename. */
  std::string new_name_;
  /*! \brief The type of action to be performed. */
  ActionType type_;
  /*! \brief Expression used to generate a new evaluator. */
  std::string expression_;
  /*! \brief Conditions used to generate a new evaluator. */
  std::vector<TagCondition> conditions_;
};
