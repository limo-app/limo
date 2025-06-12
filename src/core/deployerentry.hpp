#ifndef DEPLOYERENTRY_H
#define DEPLOYERENTRY_H

#include <json/value.h>
#include <string>
#include <vector>

class DeployerEntry {
public:
  DeployerEntry (bool isSeparator, const std::string& name)
      : isSeparator(isSeparator), name(name) {}
  bool isSeparator;
  std::string name;
  Json::Value toJson();
};

class DeployerModInfo : public DeployerEntry {
public:
  DeployerModInfo(bool isSeparator, const std::string& name, const std::string& sourceName = "", int id = -1, int enabled = 0)
      : DeployerEntry(isSeparator, name), sourceName(sourceName), id(id), enabled(enabled) {}
  std::string sourceName;
  int id;
  int enabled;
  std::vector<std::string> manual_tags;
  std::vector<std::string> auto_tags;
};

#endif
