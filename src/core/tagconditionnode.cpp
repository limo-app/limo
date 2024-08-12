#include "tagconditionnode.h"
#include <algorithm>
#include <format>
#include <ranges>
#include <regex>

namespace str = std::ranges;
namespace sfs = std::filesystem;


TagConditionNode::TagConditionNode()
{
  expression_ = "";
  invert_ = false;
  children_ = {};
  type_ = Type::empty;
  condition_ = "";
  condition_strings_ = {};
  condition_id_ = -1;
  use_regex_ = false;
}

TagConditionNode::TagConditionNode(std::string expression,
                                   const std::vector<TagCondition>& conditions) :
  expression_(expression)
{
  if(expression == "")
  {
    type_ = Type::empty;
    return;
  }
  if(!expressionIsValid(expression, conditions.size()))
    throw std::runtime_error(std::format("Invalid expression '{}'", expression));

  std::transform(expression.begin(),
                 expression.end(),
                 expression.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  removeWhitespaces(expression);
  removeEnclosingParentheses(expression);
  auto tokens = tokenize(expression);
  while(tokens.size() == 1 && expression.compare(0, 3, "not") == 0)
  {
    invert_ = !invert_;
    expression.erase(expression.begin(), expression.begin() + 3);
    removeEnclosingParentheses(expression);
    tokens = tokenize(expression);
  }
  if(tokens.size() == 1)
  {
    if(expression.find_first_not_of("0123456789") != std::string::npos)
      throw std::runtime_error(
        std::format("Error: Could not parse condition in expression '{}'", expression));
    int condition_index = std::stoi(expression);
    if(condition_index >= conditions.size())
      throw std::runtime_error(std::format(
        "Error: Condition index {} out of range in expression '{}'", condition_index, expression));
    condition_id_ = condition_index;
    type_ = conditions[condition_index].condition_type == TagCondition::Type::path
              ? Type::path_matcher
              : Type::file_matcher;
    condition_ = conditions[condition_index].search_string;
    condition_strings_ = splitString(condition_);
    invert_ = conditions[condition_index].invert ? !invert_ : invert_;
    use_regex_ = conditions[condition_index].use_regex;
    if(!use_regex_)
      std::transform(condition_.begin(),
                     condition_.end(),
                     condition_.begin(),
                     [](unsigned char c) { return std::tolower(c); });
  }
  else
  {
    type_ = containsOperator(expression, "or") ? Type::or_connector : Type::and_connector;
    for(auto [start, size] : tokens)
      children_.emplace_back(expression.substr(start, size), conditions);
  }
}

bool TagConditionNode::evaluate(const std::vector<std::pair<std::string, std::string>>& files) const
{
  if(type_ == Type::empty)
    return false;
  std::map<int, bool> results;
  return evaluateOnce(files, results);
}

bool TagConditionNode::evaluateOnce(const std::vector<std::pair<std::string, std::string>>& files,
                                    std::map<int, bool>& results) const
{
  return invert_ ? !evaluateWithoutInversion(files, results)
                 : evaluateWithoutInversion(files, results);
}

bool TagConditionNode::evaluateWithoutInversion(
  const std::vector<std::pair<std::string, std::string>>& files,
  std::map<int, bool>& results) const
{
  if(type_ == Type::file_matcher || type_ == Type::path_matcher)
  {
    if(results.contains(condition_id_))
      return results[condition_id_];

    bool result = false;
    for(const auto& [path, file_name] : files)
    {
      std::string target = type_ == Type::file_matcher ? file_name : path;
      if(use_regex_)
        result = std::regex_match(target, std::regex(condition_));
      else
      {
        std::transform(target.begin(),
                       target.end(),
                       target.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        result = wildcardMatch(target);
      }
      if(result)
        break;
    }
    results[condition_id_] = result;
    return result;
  }
  else if(type_ == Type::or_connector)
  {
    for(const auto& child : children_)
      if(child.evaluateOnce(files, results))
        return true;
    return false;
  }
  else
  {
    for(const auto& child : children_)
      if(!child.evaluateOnce(files, results))
        return false;
    return true;
  }
}

void TagConditionNode::removeEnclosingParentheses(std::string& expression)
{
  while(expression.front() == '(' && expression.back() == ')')
  {
    int level = 0;
    for(auto [i, c] : str::enumerate_view(expression))
    {
      if(c == '(')
        level++;
      else if(c == ')')
        level--;
      if(i != expression.size() - 1 && level == 0)
        return;
    }
    expression.erase(expression.begin());
    expression.erase(expression.end() - 1);
  }
}

bool TagConditionNode::expressionIsValid(std::string expression, int num_conditions)
{
  if(expression.empty())
    return false;
  std::transform(expression.begin(),
                 expression.end(),
                 expression.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  // check for invalid operators
  if(expression.find_first_not_of("notadr0123456789() ") != std::string::npos)
    return false;
  std::string expression_2 = expression;
  removeSubstring(expression_2, "and");
  removeSubstring(expression_2, "or");
  removeSubstring(expression_2, "not");
  if(std::regex_search(expression_2, std::regex("[a-zA-Z]")))
    return false;

  removeSubstring(expression, " ");
  // check for invalid parentheses
  char last_c = ' ';
  int level = 0;
  for(auto c : expression)
  {
    if(c == '(')
      level++;
    else if(c == ')')
    {
      if(last_c == '(')
        return false;
      level--;
    }
    last_c = c;
  }
  if(level != 0)
    return false;

  // check if variables exist
  const std::regex num_regex(R"(\d+)");
  auto first = std::sregex_iterator(expression.begin(), expression.end(), num_regex);
  auto last = std::sregex_iterator();
  for(auto iter = first; iter != last; iter++)
  {
    if(std::stoi(iter->str()) >= num_conditions)
      return false;
  }

  return operatorOrderIsValid(expression);
}

bool TagConditionNode::containsOperator(const std::string& expression, const std::string& op) const
{
  int level = 0;
  for(auto [i, c] : str::enumerate_view(expression))
  {
    if(c == '(')
    {
      level++;
      continue;
    }
    if(level > 0)
    {
      if(c == ')')
        level--;
      continue;
    }
    if(expression.compare(i, op.size(), op) == 0)
      return true;
  }
  return false;
}

std::vector<std::pair<int, int>> TagConditionNode::tokenize(const std::string& expression) const
{
  bool or_has_priority = containsOperator(expression, "or");
  bool and_has_priority = !or_has_priority && containsOperator(expression, "and");
  std::vector<std::pair<int, int>> tokens;
  int level = 0;
  int token_start = 0;
  for(auto [i, c] : str::enumerate_view(expression))
  {
    if(c == '(')
    {
      level++;
      continue;
    }
    if(level > 0)
    {
      if(c == ')')
        level--;
      continue;
    }
    if(or_has_priority && expression.compare(i, 2, "or") == 0)
    {
      tokens.emplace_back(token_start, i - token_start);
      token_start = i + 2;
    }
    else if(and_has_priority && expression.compare(i, 3, "and") == 0)
    {
      tokens.emplace_back(token_start, i - token_start);
      token_start = i + 3;
    }
  }
  tokens.emplace_back(token_start, expression.size() - token_start);
  return tokens;
}

void TagConditionNode::removeWhitespaces(std::string& expression) const
{
  auto pos = expression.find(" ");
  while(pos != std::string::npos)
  {
    expression.erase(pos, 1);
    pos = expression.find(" ");
  }
}

bool TagConditionNode::wildcardMatch(const std::string& target) const
{
  if(condition_.empty())
    return false;
  if(condition_.find_first_not_of("*") == std::string::npos)
    return true;

  auto condition_strings_ = splitString(condition_);
  if(condition_.front() != '*' && !target.starts_with(condition_strings_[0]) ||
     condition_.back() != '*' && !target.ends_with(condition_strings_.back()))
    return false;

  size_t target_pos = 0;
  for(const auto& search_string : condition_strings_)
  {
    if(target_pos >= target.size())
      return false;
    target_pos = target.find(search_string, target_pos);
    if(target_pos == std::string::npos)
      return false;
    target_pos += search_string.size();
  }
  return true;
}

std::vector<std::string> TagConditionNode::splitString(const std::string& input) const
{
  std::vector<std::string> splits;
  size_t pos = 0;
  size_t old_pos = 0;
  while(old_pos != input.size())
  {
    pos = input.find('*', old_pos);
    if(pos == std::string::npos)
    {
      splits.push_back(input.substr(old_pos));
      break;
    }
    if(pos - old_pos > 0)
      splits.push_back(input.substr(old_pos, pos - old_pos));
    old_pos = pos + 1;
  }
  return splits;
}

bool TagConditionNode::operatorOrderIsValid(std::string expression)
{
  constexpr int type_var = 0;
  constexpr int type_op = 1;
  constexpr int type_group = 2;
  constexpr int type_not = 3;

  TagConditionNode::removeEnclosingParentheses(expression);
  std::vector<int> token_types;
  std::vector<std::pair<int, int>> token_borders;
  int level = 0;
  int token_start = 0;
  bool is_in_group = false;
  bool is_in_var = false;
  int i = 0;
  while(i < expression.size())
  {
    char c = expression[i];
    if(is_in_var)
    {
      if(c < '0' || c > '9')
      {
        token_types.push_back(type_var);
        token_borders.emplace_back(token_start, i - token_start);
        is_in_var = false;
      }
      else
      {
        i++;
        continue;
      }
    }
    if(is_in_group)
    {
      if(c == '(')
        level++;
      else if(c == ')')
      {
        level--;
        if(level == 0)
        {
          is_in_group = false;
          token_types.push_back(type_group);
          token_borders.emplace_back(token_start, i - token_start + 1);
        }
      }
      i++;
    }
    else if(c == '(')
    {
      is_in_group = true;
      token_start = i;
      level++;
      i++;
    }
    else if(c == 'a')
    {
      token_borders.emplace_back(i, 3);
      token_types.push_back(type_op);
      i += 3;
    }
    else if(c == 'o')
    {
      token_borders.emplace_back(i, 2);
      token_types.push_back(type_op);
      i += 2;
    }
    else if(c == 'n')
    {
      token_borders.emplace_back(i, i + 2);
      token_types.push_back(type_not);
      i += 3;
    }
    else if('0' <= c && c <= '9')
    {
      if(!is_in_var)
        token_start = i;
      is_in_var = true;
      i++;
    }
    else
      i++;
  }
  if(is_in_var)
  {
    token_types.push_back(type_var);
    token_borders.emplace_back(token_start, i - token_start);
  }

  int prev_token = type_op;
  for(int token : token_types)
  {
    if(token == type_op && (prev_token == type_not || prev_token == type_op))
      return false;
    if(token == type_var && (prev_token == type_var || prev_token == type_group))
      return false;
    if(token == type_group && (prev_token == type_var || prev_token == type_group))
      return false;
    if(token == type_not && ((prev_token == type_var || prev_token == type_group)))
      return false;
    prev_token = token;
  }
  if(token_types.back() == type_not || token_types.back() == type_op)
    return false;

  for(const auto& [token, borders] : str::zip_view(token_types, token_borders))
  {
    if(token != type_group)
      continue;
    const auto [start, len] = borders;
    if(!operatorOrderIsValid(expression.substr(start, len)))
      return false;
  }
  return true;
}

void TagConditionNode::removeSubstring(std::string& string, std::string substring)
{
  const size_t length = substring.length();
  for(auto pos = string.find(substring); pos != std::string::npos; pos = string.find(substring))
    string.erase(pos, length);
}
