#include "manualtag.h"
#include "parseerror.h"

namespace str = std::ranges;


ManualTag::ManualTag(std::string name)
{
  name_ = name;
}

ManualTag::ManualTag(const Json::Value& json)
{
  if(!json.isMember("name"))
    throw ParseError("Tag name is missing.");
  name_ = json["name"].asString();

  if(json.isMember("mod_ids"))
  {
    for(const auto& mod : json["mod_ids"])
      mods_.push_back(mod.asInt());
  }
}

void ManualTag::addMod(int mod_id)
{
  auto iter = str::find(mods_, mod_id);
  if(iter == mods_.end())
    mods_.push_back(mod_id);
}

void ManualTag::removeMod(int mod_id)
{
  auto iter = str::find(mods_, mod_id);
  if(iter != mods_.end())
    mods_.erase(iter);
}

void ManualTag::setMods(const std::vector<int> mods)
{
  mods_ = mods;
}

Json::Value ManualTag::toJson() const
{
  Json::Value json;
  json["name"] = name_;
  for(int i = 0; i < mods_.size(); i++)
    json["mod_ids"][i] = mods_[i];
  return json;
}

bool ManualTag::operator==(const std::string& name) const
{
  return this->name_ == name;
}

bool ManualTag::operator==(const ManualTag& other) const
{
  return this->name_ == other.name_;
}
