#include "rootlevelcondition.h"
#include "../core/wildcardmatching.h"
#include <queue>
#include <regex>


RootLevelCondition::RootLevelCondition(MatcherType matcher_type,
                                       TargetType target_type,
                                       bool case_invariant,
                                       bool stop_on_branch,
                                       const std::string& expression,
                                       int level_offset) :
  matcher_type_(matcher_type), target_type_(target_type), case_invariant_matching_(case_invariant),
  stop_on_branch_(stop_on_branch), expression_(expression), level_offset_(level_offset)
{}

RootLevelCondition::RootLevelCondition(const Json::Value& json)
{
  const std::vector<std::string> required_keys = { JSON_MATCHER_TYPE_KEY,
                                                   JSON_EXPRESSION_KEY,
                                                   JSON_TARGET_TYPE_KEY };
  for(const auto& key : required_keys)
    if(!json.isMember(key))
      throw std::runtime_error(std::format("Missing json key: '{}'.", key));

  const std::string matcher_type = json[JSON_MATCHER_TYPE_KEY].asString();
  if(matcher_type == JSON_MATCHER_SIMPLE_VALUE)
    matcher_type_ = simple;
  else if(matcher_type == JSON_MATCHER_REGEX_VALUE)
    matcher_type_ = regex;
  else
    throw std::runtime_error(std::format("Invalid matcher type: '{}'.", matcher_type));

  const std::string target_type = json[JSON_TARGET_TYPE_KEY].asString();
  if(target_type == JSON_TARGET_ANY_VALUE)
    target_type_ = any;
  else if(target_type == JSON_TARGET_FILE_VALUE)
    target_type_ = file;
  else if(target_type == JSON_TARGET_DIRECTORY_VALUE)
    target_type_ = directory;
  else
    throw std::runtime_error(std::format("Invalid target type: '{}'.", target_type));

  expression_ = json[JSON_EXPRESSION_KEY].asString();
  case_invariant_matching_ = false;
  if(json.isMember(JSON_CASE_INVARIANT_KEY))
    case_invariant_matching_ = json[JSON_CASE_INVARIANT_KEY].asBool();
  stop_on_branch_ = true;
  if(json.isMember(JSON_STOP_KEY))
    stop_on_branch_ = json[JSON_STOP_KEY].asBool();
  level_offset_ = 0;
  if(json.isMember(JSON_OFFSET_KEY))
    level_offset_ = json[JSON_OFFSET_KEY].asInt();
}

std::optional<int> RootLevelCondition::detectRootLevel(QTreeWidgetItem* root_node,
                                                       int cur_level) const
{
  if(root_node->childCount() == 0)
    return {};

  auto compare_levels = [](const auto& pair_l, const auto& pair_r)
  { return pair_l.second > pair_r.second; };
  std::priority_queue<std::pair<QTreeWidgetItem*, int>,
                      std::vector<std::pair<QTreeWidgetItem*, int>>,
                      decltype(compare_levels)>
    remaining_nodes{ compare_levels };

  for(int i = 0; i < root_node->childCount(); i++)
    remaining_nodes.push({ root_node->child(i), cur_level });

  std::regex expression_regex;
  if(matcher_type_ == regex)
    expression_regex.assign(expression_);

  while(!remaining_nodes.empty())
  {
    auto [node, level] = remaining_nodes.top();
    remaining_nodes.pop();
    const bool is_directory = node->data(0, Qt::UserRole).toBool();
    if(target_type_ == any || target_type_ == file && !is_directory ||
       target_type_ == directory && is_directory)
    {
      const std::string cur_text = case_invariant_matching_ ? node->text(0).toLower().toStdString()
                                                            : node->text(0).toStdString();
      if(matcher_type_ == regex)
      {
        std::smatch match;
        if(std::regex_match(cur_text, match, expression_regex))
          return level - level_offset_;
      }
      else if(wildcardMatch(cur_text, expression_))
        return level - level_offset_;
    }

    for(int i = 0; i < node->childCount(); i++)
      remaining_nodes.push({ node->child(i), level + 1 });
  }

  return {};
}
