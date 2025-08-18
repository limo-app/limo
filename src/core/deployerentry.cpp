#include "deployerentry.hpp"
#include <json/config.h>


Json::Value DeployerEntry::toJson()
{
  if (!isSeparator) {
    return static_cast<DeployerModInfo *>(this)->toJson();
  }
  Json::Value json_object;
  json_object["name"] = name;
  json_object["expanded"] = isExpanded;
  return json_object;
}

Json::Value DeployerModInfo::toJson()
{
  Json::Value json_object;
  json_object["id"] = id;
  json_object["status"] = enabled;
  return json_object;
}

bool DeployerModInfo::operator==(const DeployerModInfo& other) const {
  return this->isSeparator == other.isSeparator
    && this->id == other.id
    && this->enabled == other.enabled
    && this->name == other.name;
}

bool DeployerEntry::operator==(const DeployerEntry& other) const {
  if (!isSeparator && !other.isSeparator) {
    return static_cast<const DeployerModInfo *>(this)->operator==(
      static_cast<const DeployerModInfo &>(other));
  }
  return this->name == other.name
    && this->isSeparator == other.isSeparator;
}
