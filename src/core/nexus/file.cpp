#include "file.h"
#include "../parseerror.h"

using namespace nexus;


File::File(const std::string& http_body)
{
  Json::Value json_body;
  Json::Reader reader;
  bool success = reader.parse(http_body.c_str(), json_body);
  if(!success)
    throw ParseError("Failed to parse response from NexusMods.");

  init(json_body);
}

File::File(const Json::Value& json_body)
{
  init(json_body);
}

void File::init(const Json::Value& json_body)
{
  id_0 = json_body["id"][0].asInt64();
  id_1 = json_body["id"][1].asInt64();
  uid = json_body["uid"].asInt64();
  file_id = json_body["file_id"].asInt64();
  name = json_body["name"].asString();
  version = json_body["version"].asString();
  category_id = json_body["category_id"].asInt64();
  category_name = json_body["category_name"].asString();
  is_primary = json_body["is_primary"].asBool();
  size = json_body["size"].asInt64();
  file_name = json_body["file_name"].asString();
  uploaded_time = json_body["uploaded_timestamp"].asInt64();
  mod_version = json_body["mod_version"].asString();
  external_virus_scan_url = json_body["external_virus_scan_url"].asString();
  description = json_body["description"].asString();
  size_kb = json_body["size_kb"].asInt64();
  size_in_bytes = json_body["size_in_bytes"].asInt64();
  changelog_html = json_body["changelog_html"].asString();
  content_preview_link = json_body["content_preview_link"].asString();
}
