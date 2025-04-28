#include "bg3plugin.h"
#include <algorithm>

namespace str = std::ranges;


Bg3Plugin::Bg3Plugin(const std::string& xml_string) : xml_string_(xml_string)
{
  pugi::xml_document xml_doc;
  xml_doc.load_string(xml_string.c_str());
  pugi::xml_node root_node = xml_doc.child("save")
                               .find_child_by_attribute("id", "Config")
                               .find_child_by_attribute("id", "root")
                               .child("children");
  pugi::xml_node module_node = root_node.find_child_by_attribute("id", "ModuleInfo");

  for(const auto& child : module_node.children())
  {
    if(std::string("Name").compare(child.attribute("id").value()) == 0)
      name_ = child.attribute("value").value();
    else if(std::string("UUID").compare(child.attribute("id").value()) == 0)
      uuid_ = child.attribute("value").value();
    else if(std::string("Version64").compare(child.attribute("id").value()) == 0)
      version_ = child.attribute("value").value();
    else if(std::string("Version").compare(child.attribute("id").value()) == 0)
      version_ = child.attribute("value").value();
    else if(std::string("Description").compare(child.attribute("id").value()) == 0)
      description_ = child.attribute("value").value();
    else if(std::string("Folder").compare(child.attribute("id").value()) == 0)
      directory_ = child.attribute("value").value();
  }

  pugi::xml_node dependencies_node =
    root_node.find_child_by_attribute("id", "Dependencies").child("children");
  for(const auto& dependency : dependencies_node)
  {
    const std::string uuid =
      dependency.find_child_by_attribute("id", "UUID").attribute("value").value();
    const std::string name =
      dependency.find_child_by_attribute("id", "Name").attribute("value").value();
    if(!uuid.empty() && !BG3_VANILLA_UUIDS.contains(uuid))
      dependencies_.emplace_back(uuid, name);
  }
}

std::string Bg3Plugin::getUuid() const
{
  return uuid_;
}

std::string Bg3Plugin::getVersion() const
{
  return version_;
}

std::string Bg3Plugin::getDirectory() const
{
  return directory_;
}

std::string Bg3Plugin::getName() const
{
  return name_;
}

std::string Bg3Plugin::getDescription() const
{
  return description_;
}

std::vector<std::pair<std::string, std::string>> Bg3Plugin::getDependencies() const
{
  return dependencies_;
}

bool Bg3Plugin::hasDependency(const std::string& uuid)
{
  return str::any_of(dependencies_, [uuid](const auto& p) { return p.first == uuid; });
}

std::vector<std::pair<std::string, std::string>> Bg3Plugin::getMissingDependencies(
  const std::set<std::string>& plugin_uuids)
{
  std::vector<std::pair<std::string, std::string>> missing_dependencies;
  for(const auto& [uuid, name] : dependencies_)
  {
    if(!plugin_uuids.contains(uuid))
      missing_dependencies.emplace_back(uuid, name);
  }
  return missing_dependencies;
}

std::string Bg3Plugin::getXmlString() const
{
  return xml_string_;
}

std::string Bg3Plugin::toXmlPluginString() const
{
  return std::string(R"(<node id="ModuleShortDesc">)") + "\n" +
         R"(<attribute id="Folder" type="LSString" value=")" + directory_ + "\"/>\n" +
         R"(<attribute id="MD5" type="LSString" value=""/>)" + "\n" +
         R"(<attribute id="Name" type="LSString" value=")" + name_ + "\"/>\n" +
         R"(<attribute id="UUID" type="FixedString" value=")" + uuid_ + "\"/>\n" +
         R"(<attribute id="Version64" type="int64" value=")" + version_ + "\"/>\n" + "</node>\n";
}

std::string Bg3Plugin::toXmlLoadorderString() const
{
  return std::string(R"(<node id="Module">)") + "\n" +
         R"(<attribute id="UUID" type="FixedString" value=")" + uuid_ + "\"/>\n" + "</node>\n";
}

void Bg3Plugin::addToXmlModsNode(pugi::xml_node& root) const
{
  auto desc = root.append_child("node");
  desc.append_attribute("id") = "ModuleShortDesc";

  auto folder = desc.append_child("attribute");
  folder.append_attribute("id") = "Folder";
  folder.append_attribute("type") = "LSString";
  folder.append_attribute("value") = directory_.c_str();

  auto md5 = desc.append_child("attribute");
  md5.append_attribute("id") = "MD5";
  md5.append_attribute("type") = "LSString";
  md5.append_attribute("value") = "";

  auto name = desc.append_child("attribute");
  name.append_attribute("id") = "Name";
  name.append_attribute("type") = "LSString";
  name.append_attribute("value") = name_.c_str();

  auto uuid = desc.append_child("attribute");
  uuid.append_attribute("id") = "UUID";
  uuid.append_attribute("type") = "FixedString";
  uuid.append_attribute("value") = uuid_.c_str();

  auto version = desc.append_child("attribute");
  version.append_attribute("id") = "Version64";
  version.append_attribute("type") = "int64";
  version.append_attribute("value") = version_.c_str();
}

void Bg3Plugin::addToXmlOrderNode(pugi::xml_node& root) const
{
  auto module = root.append_child("node");
  module.append_attribute("id") = "Module";

  auto uuid = module.append_child("attribute");
  uuid.append_attribute("id") = "UUID";
  uuid.append_attribute("type") = "FixedString";
  uuid.append_attribute("value") = uuid_.c_str();
}

bool Bg3Plugin::isValidPlugin(const std::string& xml_string)
{
  pugi::xml_document xml_doc;
  xml_doc.load_string(xml_string.c_str());
  pugi::xml_node uuid_node = xml_doc.child("save")
                               .find_child_by_attribute("id", "Config")
                               .find_child_by_attribute("id", "root")
                               .child("children")
                               .find_child_by_attribute("id", "ModuleInfo")
                               .find_child_by_attribute("id", "UUID");
  if(uuid_node.empty())
    return false;
  const std::string uuid = uuid_node.attribute("value").value();
  return !uuid.empty() && !BG3_VANILLA_UUIDS.contains(uuid);
}
