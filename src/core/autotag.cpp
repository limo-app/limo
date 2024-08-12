#include "autotag.h"
#include "parseerror.h"
#include <format>
#include <ranges>

namespace sfs = std::filesystem;
namespace str = std::ranges;


AutoTag::AutoTag(const std::string& name,
                 const std::string& expression,
                 const std::vector<TagCondition>& conditions) :
  expression_(expression), conditions_(conditions), evaluator_(expression, conditions)
{
  name_ = name;
}

AutoTag::AutoTag(const Json::Value& json)
{
  if(!json.isMember("name"))
    throw ParseError("Tag name is missing.");
  name_ = json["name"].asString();

  if(json.isMember("mod_ids"))
  {
    for(const auto& mod : json["mod_ids"])
      mods_.push_back(mod.asInt());
  }

  if(!json.isMember("expression"))
    throw ParseError("Auto-Tag expression is missing.");
  expression_ = json["expression"].asString();

  if(!json.isMember("conditions"))
    throw ParseError("Auto-Tag conditions are missing.");
  for(const auto& json_condition : json["conditions"])
  {
    TagCondition condition;
    if(!json_condition.isMember("invert"))
      throw ParseError("Auto-Tag condition invert flag is missing.");
    condition.invert = json_condition["invert"].asBool();

    if(!json_condition.isMember("use_regex"))
      throw ParseError("Auto-Tag condition use_regex flag is missing.");
    condition.use_regex = json_condition["use_regex"].asBool();

    if(!json_condition.isMember("search_string"))
      throw ParseError("Auto-Tag search_string is missing.");
    condition.search_string = json_condition["search_string"].asString();

    if(!json_condition.isMember("condition_type"))
      throw ParseError("Auto-Tag condition_type is missing.");
    condition.condition_type = json_condition["condition_type"].asString() == "file_name"
                                 ? TagCondition::Type::file_name
                                 : TagCondition::Type::path;
    conditions_.push_back(condition);
  }
  if(!TagConditionNode::expressionIsValid(expression_, conditions_.size()))
    throw ParseError(std::format("Invalid auto tag expression \"{}\".", expression_));
  evaluator_ = TagConditionNode(expression_, conditions_);
}

void AutoTag::setEvaluator(const std::string& expression,
                           const std::vector<TagCondition>& conditions)
{
  expression_ = expression;
  conditions_ = conditions;
  evaluator_ = TagConditionNode(expression, conditions_);
}

Json::Value AutoTag::toJson() const
{
  Json::Value json;
  for(int i = 0; i < mods_.size(); i++)
    json["mod_ids"][i] = mods_[i];

  json["expression"] = expression_;
  json["name"] = name_;

  for(const auto& [index, condition] : str::enumerate_view(conditions_))
  {
    const int i = index;
    json["conditions"][i]["invert"] = condition.invert;
    json["conditions"][i]["use_regex"] = condition.use_regex;
    json["conditions"][i]["search_string"] = condition.search_string;
    json["conditions"][i]["condition_type"] =
      condition.condition_type == TagCondition::Type::file_name ? "file_name" : "path";
  }
  return json;
}

bool AutoTag::operator==(const std::string& name) const
{
  return name_ == name;
}

std::string AutoTag::getExpression() const
{
  return expression_;
}

std::vector<TagCondition> AutoTag::getConditions() const
{
  return conditions_;
}

int AutoTag::getNumConditions() const
{
  return conditions_.size();
}
