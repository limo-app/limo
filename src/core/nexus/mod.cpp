#include "mod.h"
#include "../parseerror.h"
#include <json/json.h>

using namespace nexus;


Mod::Mod(const std::string& http_body)
{
  Json::Value json_body;
  Json::Reader reader;
  bool success = reader.parse(http_body.c_str(), json_body);
  if(!success)
    throw ParseError("Failed to parse response from NexusMods.");

  init(json_body);
}

Mod::Mod(const Json::Value& json_body)
{
  init(json_body);
}

void Mod::init(const Json::Value& json_body)
{
  name = json_body["name"].asString();
  summary = json_body["summary"].asString();
  description = json_body["description"].asString();
  picture_url = json_body["picture_url"].asString();
  mod_downloads = json_body["mod_downloads"].asInt64();
  mod_unique_downloads = json_body["mod_unique_downloads"].asInt64();
  uid = json_body["uid"].asInt64();
  mod_id = json_body["mod_id"].asInt64();
  game_id = json_body["game_id"].asInt64();
  allow_rating = json_body["allow_rating"].asBool();
  domain_name = json_body["domain_name"].asString();
  category_id = json_body["category_id"].asInt64();
  version = json_body["version"].asString();
  endorsement_count = json_body["endorsement_count"].asInt64();
  created_time = json_body["created_timestamp"].asInt64();
  updated_time = json_body["updated_timestamp"].asInt64();
  author = json_body["author"].asString();
  uploaded_by = json_body["uploaded_by"].asString();
  uploaded_users_profile_url = json_body["uploaded_users_profile_url"].asString();
  contains_adult_content = json_body["contains_adult_content"].asBool();
  status = json_body["status"].asString();
  available = json_body["available"].asBool();
  user_member_id = json_body["user"]["member_id"].asInt64();
  user_member_group_id = json_body["user"]["member_group_id"].asInt64();
  user_name = json_body["user"]["name"].asString();
  endorsement_status = json_body["endorsement"]["endorse_status"].asString();
}
