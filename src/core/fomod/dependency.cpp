#include "dependency.h"
#include "../log.h"
#include "../pathutils.h"

using namespace fomod;
namespace sfs = std::filesystem;
namespace pu = path_utils;


Dependency::Dependency(pugi::xml_node source)
{
  if(!source)
  {
    type_ = dummy_node;
    return;
  }
  const std::string name(source.name());
  if(name == "dependencies" || name == "moduleDependencies")
  {
    type_ = source.attribute("operator").value() == std::string("Or") ? or_node : and_node;
    std::map<std::string, std::pair<std::string, pugi::xml_node>> file_dependencies;
    for(auto child : source.children())
    {
      if(type_ == or_node && child && std::string(child.name()) == "fileDependency")
      {
        const std::string target = child.attribute("file").value();
        if(!file_dependencies.contains(target))
          file_dependencies[target] = { child.attribute("state").value(), child };
        else
        {
          const std::string child_state = child.attribute("state").value();
          if(child_state == "Active" && file_dependencies[target].first != "Active")
            file_dependencies[target] = { "Active", child };
        }
      }
      else
        children_.emplace_back(child);
    }
    for(const auto& [target, pair] : file_dependencies)
      children_.emplace_back(pair.second);
  }
  else if(name == "fileDependency")
  {
    type_ = file_leaf;
    target_ = source.attribute("file").value();
    state_ = source.attribute("state").value();
  }
  else if(name == "flagDependency")
  {
    type_ = flag_leaf;
    target_ = source.attribute("flag").value();
    state_ = source.attribute("value").value();
  }
  else if(name == "gameDependency")
  {
    type_ = game_version_leaf;
    target_ = source.attribute("version").value();
  }
  else if(name == "fommDependency")
  {
    type_ = fomm_version_leaf;
    target_ = source.attribute("version").value();
  }
}

Dependency::Dependency()
{
  type_ = dummy_node;
}

bool Dependency::evaluate(const sfs::path& target_path,
                          const std::map<std::string, std::string>& flags,
                          std::function<bool(std::string)> eval_game_version,
                          std::function<bool(std::string)> eval_fomm_version) const
{
  if(type_ == and_node)
  {
    if(children_.empty())
      return true;
    for(const auto& child : children_)
    {
      if(!child.evaluate(target_path, flags, eval_game_version, eval_fomm_version))
        return false;
    }
    return true;
  }
  else if(type_ == or_node)
  {
    if(children_.empty())
      return true;
    for(const auto& child : children_)
    {
      if(child.evaluate(target_path, flags, eval_game_version, eval_fomm_version))
        return true;
    }
    return false;
  }
  else if(type_ == file_leaf)
  {
    const bool exists = pu::pathExists(target_, target_path) ? true : false;
    if(state_ == "Active")
      return exists;
    return !exists;
  }
  else if(type_ == flag_leaf)
  {
    if(!flags.contains(target_))
    {
      // Default values for unset flags are not defined in the FOMOD spec. This assumes that
      // a test against an empty string attempts to check if a flag is not set.
      if(state_.empty())
      {
        Log::warning(
          "The FOMOD file attempted to compare the value of a flag to an empty string. "
          "This installer assumes that the mod author meant to check if the flag was not set. "
          "Please ensure that the mod is installed correctly.");
        return true;
      }
      return false;
    }
    return flags.at(target_) == state_;
  }
  else if(type_ == game_version_leaf)
    return eval_game_version(target_);
  else if(type_ == fomm_version_leaf)
    return eval_fomm_version(target_);
  return true;
}

std::string Dependency::toString() const
{
  if(type_ == file_leaf)
    return "(File '" + target_ + "' is '" + state_ + "')";
  else if(type_ == flag_leaf)
    return "(Flag '" + target_ + "' is '" + state_ + "')";
  else if(type_ == game_version_leaf)
    return "(Game version == '" + target_ + "')";
  else if(type_ == fomm_version_leaf)
    return "(Fomm version == '" + target_ + "')";
  else
  {
    std::string op = type_ == or_node ? "OR" : "AND";
    std::string chain = "( ";
    for(int i = 0; i < children_.size(); i++)
    {
      chain += children_.at(i).toString();
      if(i < children_.size() - 1)
        chain += " " + op + " ";
    }
    return chain + " )";
  }
}
