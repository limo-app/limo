#ifndef DEPLOYERENTRY_H
#define DEPLOYERENTRY_H

#include <json/value.h>
#include <string>
#include <vector>

class DeployerEntry {
public:
  DeployerEntry (bool isSeparator, const std::string& name, int id = -2)
      : isSeparator(isSeparator), name(name), id(id) {}
  bool isSeparator;
  std::string name;
  bool isExpanded = false;
  int id;
  Json::Value toJson();
  bool operator==(const DeployerEntry& other) const;
};

class DeployerModInfo : public DeployerEntry {
public:
  DeployerModInfo(bool isSeparator, const std::string& name, const std::string& sourceName = "", int id = -1, bool enabled = false)
      : DeployerEntry(isSeparator, name, id),
        sourceName(sourceName),
        enabled(enabled),
        manual_tags(std::vector<std::string>()),
        auto_tags(std::vector<std::string>()) {}
  std::string sourceName;
  int enabled;
  std::vector<std::string> manual_tags;
  std::vector<std::string> auto_tags;
  Json::Value toJson();
  bool operator==(const DeployerModInfo& other) const;
};

#endif
