#include "editautotagaction.h"

EditAutoTagAction::EditAutoTagAction(const std::string& name, ActionType type)
{
  name_ = name;
  type_ = type;
}

EditAutoTagAction::EditAutoTagAction(const std::string& name, const std::string& new_name)
{
  name_ = name;
  new_name_ = new_name;
  type_ = ActionType::rename;
}

EditAutoTagAction::EditAutoTagAction(const std::string& name,
                                     const std::string& expression,
                                     const std::vector<TagCondition>& conditions)
{
  name_ = name;
  expression_ = expression;
  conditions_ = conditions;
  type_ = ActionType::change_evaluator;
}

std::string EditAutoTagAction::getName() const
{
  return name_;
}

std::string EditAutoTagAction::getNewName() const
{
  return new_name_;
}

EditAutoTagAction::ActionType EditAutoTagAction::getType() const
{
  return type_;
}

std::string EditAutoTagAction::getExpression() const
{
  return expression_;
}

std::vector<TagCondition> EditAutoTagAction::getConditions() const
{
  return conditions_;
}
