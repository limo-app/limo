#include "mod.h"

Mod::Mod(int id,
         const std::string& name,
         const std::string& version,
         const std::time_t& time,
         const std::filesystem::path& source_l,
         const std::string& source_r,
         const std::time_t& time_r,
         unsigned long size,
         const std::time_t& suppress_time) :
  id(id), name(std::move(name)), version(std::move(version)), install_time(time),
  local_source(source_l), remote_source(source_r), remote_update_time(time_r), size_on_disk(size),
  suppress_update_time(suppress_time)
{}

Mod::Mod(const Json::Value& json)
{
  Mod(json["id"].asInt(),
      json["name"].asString(),
      json["version"].asString(),
      json["install_time"].asInt64(),
      json["local_source"].asString(),
      json["remote_source"].asString(),
      json["remote_update_time"].asInt64(),
      json["size_on_disk"].asInt64(),
      json["suppress_update_time"].asInt64());
}

Json::Value Mod::toJson() const
{
  Json::Value json;
  json["id"] = id;
  json["name"] = name;
  json["version"] = version;
  json["install_time"] = install_time;
  json["local_source"] = local_source.string();
  json["remote_source"] = remote_source;
  json["remote_update_time"] = remote_update_time;
  json["size_on_disk"] = size_on_disk;
  json["suppress_update_time"] = suppress_update_time;
  return json;
}

bool Mod::operator==(const Mod& other) const
{
  return id == other.id;
}

bool Mod::operator<(const Mod& other) const
{
  return id < other.id;
}
