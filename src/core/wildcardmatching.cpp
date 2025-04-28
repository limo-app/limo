#include "wildcardmatching.h"


bool wildcardMatch(const std::string& target, const std::string& expression)
{
  if(expression.empty())
    return false;
  if(expression.find_first_not_of("*") == std::string::npos)
    return true;

  auto expressionstrings_ = splitString(expression);
  if(expression.front() != '*' && !target.starts_with(expressionstrings_[0]) ||
     expression.back() != '*' && !target.ends_with(expressionstrings_.back()))
    return false;

  size_t target_pos = 0;
  for(const auto& search_string : expressionstrings_)
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

std::vector<std::string> splitString(const std::string& input)
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
