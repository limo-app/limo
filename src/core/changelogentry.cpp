#include "changelogentry.h"

ChangelogEntry::ChangelogEntry(const Json::Value& json)
{
  type_ = static_cast<ChangeType>(json["type"].asInt());
  short_description_ = json["short_description"].asString();
  if(json.isMember("long_description"))
    long_description_ = json["long_description"].asString();
  if(json.isMember("issue"))
    issue_ = json["issue"].asInt();
  else
    issue_ = -1;
  if(json.isMember("pull_request"))
    pull_request_ = json["pull_request"].asInt();
  else
    pull_request_ = -1;
}

ChangelogEntry::ChangeType ChangelogEntry::getType() const
{
  return type_;
}

std::string ChangelogEntry::getShortDescription() const
{
  return short_description_;
}

std::string ChangelogEntry::getLongDescription() const
{
  return long_description_;
}

int ChangelogEntry::getIssue() const
{
  return issue_;
}

int ChangelogEntry::getPullRequest() const
{
  return pull_request_;
}

bool ChangelogEntry::operator<(const ChangelogEntry& other) const
{
  return static_cast<int>(type_) < static_cast<int>(other.type_);
}
