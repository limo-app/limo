#include "mod.h"

Mod::Mod(int id,
         const std::string& name,
         const std::string& version,
         const std::time_t& time,
         const std::filesystem::path& source_l,
         const std::string& source_r,
         const std::time_t& time_r,
         unsigned long size,
         const std::time_t& suppress_time,
         long remote_mod_id,
         long remote_file_id,
         ImportModInfo::RemoteType remote_type) :
  id(id), name(std::move(name)), version(std::move(version)), install_time(time),
  local_source(source_l), remote_source(source_r), remote_update_time(time_r), size_on_disk(size),
  suppress_update_time(suppress_time), remote_mod_id(remote_mod_id), remote_file_id(remote_file_id),
  remote_type(remote_type)
{}

Mod::Mod(const Mod& other)
{
  id = other.id;
  name = other.name;
  version = other.version;
  install_time = other.install_time;
  local_source = other.local_source;
  remote_source = other.remote_source;
  remote_update_time = other.remote_update_time;
  size_on_disk = other.size_on_disk;
  suppress_update_time = other.suppress_update_time;
  remote_mod_id = other.remote_mod_id;
  remote_file_id = other.remote_file_id;
  remote_type = other.remote_type;
}

Mod::Mod(const Json::Value& json)
{
  long remote_mod_id = -1;
  if(json.isMember("remote_mod_id"))
    remote_mod_id = json["remote_mod_id"].asInt64();
  long remote_file_id = -1;
  if(json.isMember("remote_file_id"))
    remote_file_id = json["remote_file_id"].asInt64();
  ImportModInfo::RemoteType remote_type = ImportModInfo::RemoteType::local;
  if(json.isMember("remote_type"))
    remote_type = static_cast<ImportModInfo::RemoteType>(json["remote_type"].asInt());
  id = json["id"].asInt();
  name = json["name"].asString();
  version = json["version"].asString();
  install_time = json["install_time"].asInt64();
  local_source = json["local_source"].asString();
  remote_source = json["remote_source"].asString();
  remote_update_time = json["remote_update_time"].asInt64();
  size_on_disk = json["size_on_disk"].asInt64();
  suppress_update_time = json["suppress_update_time"].asInt64();
  remote_mod_id = remote_mod_id;
  remote_file_id = remote_file_id;
  remote_type = remote_type;
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
  json["remote_mod_id"] = remote_mod_id;
  json["remote_file_id"] = remote_file_id;
  json["remote_type"] = static_cast<int>(remote_type);
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
