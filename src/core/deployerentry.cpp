#include "deployerentry.hpp"
#include <json/config.h>


Json::Value DeployerEntry::toJson()
{
  Json::Value json_object;
  json_object["name"] = name;
  return json_object;
}

Json::Value DeployerModInfo::toJson()
{
  Json::Value json_object;
  // json_object["name"] = name;
  json_object["id"] = id;
  json_object["status"] = enabled;
  // json_object["source_mod_name"] = sourceName;
  json_object["manual_tags"];
  json_object["auto_tags"];

  for (auto tag : manual_tags) {
    json_object["manual_tags"].append(tag);
  }
  for (auto tag : auto_tags) {
    json_object["auto_tags"].append(tag);
  }
  return json_object;
}
