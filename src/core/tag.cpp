#include "tag.h"

namespace str = std::ranges;


std::string Tag::getName() const
{
  return name_;
}

void Tag::setName(const std::string& name)
{
  name_ = name;
}

std::vector<int> Tag::getMods() const
{
  return mods_;
}

int Tag::getNumMods() const
{
  return mods_.size();
}

bool Tag::hasMod(int mod_id) const
{
  return str::find(mods_, mod_id) != mods_.end();
}
