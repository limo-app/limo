#include "editmanualtagaction.h"

EditManualTagAction::EditManualTagAction(const std::string& name,
                                         ActionType type,
                                         const std::string& new_name) :
  name_(name), type_(type), new_name_(new_name)
{}

std::string EditManualTagAction::getName() const
{
  return name_;
}

std::string EditManualTagAction::getNewName() const
{
  return new_name_;
}

EditManualTagAction::ActionType EditManualTagAction::getType() const
{
  return type_;
}
