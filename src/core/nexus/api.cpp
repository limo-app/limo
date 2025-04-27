#include "api.h"
#include "../parseerror.h"
#include <iostream>
#include <json/json.h>
#include <ranges>
#include <regex>

using namespace nexus;
namespace str = std::ranges;


void Api::setApiKey(const std::string& api_key)
{
  api_key_ = api_key;
}

bool Api::isInitialized()
{
  return !api_key_.empty();
}

Mod Api::getMod(const std::string& mod_url)
{
  auto domain_and_mod = extractDomainAndModId(mod_url);
  if(!domain_and_mod)
    throw std::runtime_error(std::format("Could not parse mod URL: \"{}\".", mod_url));
  return getMod(domain_and_mod->first, domain_and_mod->second);
}

Mod Api::getMod(const std::string& domain_name, long mod_id)
{
  cpr::Response response =
    cpr::Get(cpr::Url(std::format(
               "https://api.nexusmods.com/v1/games/{}/mods/{}.json", domain_name, mod_id)),
             cpr::Header{ { "apikey", api_key_ } });
  if(response.status_code != 200)
    throw std::runtime_error(
      std::format("Failed to get data for mod with id {} from NexusMods. Response code was {}",
                  mod_id,
                  response.status_code));
  return { response.text };
}

void Api::trackMod(const std::string& mod_url)
{
  auto domain_and_mod = extractDomainAndModId(mod_url);
  if(!domain_and_mod)
    throw std::runtime_error(std::format("Could not parse mod URL: \"{}\".", mod_url));
  const cpr::Response response =
    cpr::Post(cpr::Url("https://api.nexusmods.com/v1/user/tracked_mods.json"),
              cpr::Header{ { "apikey", api_key_ } },
              cpr::Parameters{ { "domain_name", domain_and_mod->first },
                               { "mod_id", std::to_string(domain_and_mod->second) } });
}

void Api::untrackMod(const std::string& mod_url)
{
  auto domain_and_mod = extractDomainAndModId(mod_url);
  if(!domain_and_mod)
    throw std::runtime_error(std::format("Could not parse mod URL: \"{}\".", mod_url));
  const cpr::Response response =
    cpr::Delete(cpr::Url("https://api.nexusmods.com/v1/user/tracked_mods.json"),
                cpr::Header{ { "apikey", api_key_ } },
                cpr::Parameters{ { "domain_name", domain_and_mod->first },
                                 { "mod_id", std::to_string(domain_and_mod->second) } });
}

std::vector<Mod> Api::getTrackedMods()
{
  cpr::Response response = cpr::Get(cpr::Url("https://api.nexusmods.com/v1/user/tracked_mods.json"),
                                    cpr::Header{ { "apikey", api_key_ } });
  if(response.status_code != 200)
    throw std::runtime_error(std::format(
      "Failed to get tracked mods from NexusMods. Response code was: {}", response.status_code));

  Json::Value json_body;
  Json::Reader reader;
  bool success = reader.parse(response.text.c_str(), json_body);
  if(!success)
    throw ParseError("Failed to parse response from NexusMods.");

  std::vector<Mod> mods;
  for(int i = 0; i < json_body.size(); i++)
    mods.push_back(
      getMod(json_body[i]["domain_name"].asString(), json_body[i]["mod_id"].asInt64()));
  return mods;
}

std::vector<File> Api::getModFiles(const std::string& mod_url)
{
  auto domain_and_mod = extractDomainAndModId(mod_url);
  if(!domain_and_mod)
    throw std::runtime_error(std::format("Could not parse mod URL: \"{}\".", mod_url));

  const auto [domain_name, mod_id] = *domain_and_mod;
  cpr::Response response =
    cpr::Get(cpr::Url(std::format(
               "https://api.nexusmods.com/v1/games/{}/mods/{}/files.json", domain_name, mod_id)),
             cpr::Header{ { "apikey", api_key_ } });
  if(response.status_code != 200)
    throw std::runtime_error(
      std::format("Failed to get mod files for mod with id {} from NexusMods. Response code was {}",
                  mod_id,
                  response.status_code));

  Json::Value json_body;
  Json::Reader reader;
  bool success = reader.parse(response.text.c_str(), json_body);
  if(!success)
    throw ParseError("Failed to parse response from NexusMods.");

  std::vector<File> files;
  for(int i = 0; i < json_body["files"].size(); i++)
    files.emplace_back(json_body["files"][i]);
  return files;
}

std::string Api::getDownloadUrl(const std::string& mod_url, long file_id)
{
  auto domain_and_mod = extractDomainAndModId(mod_url);
  if(!domain_and_mod)
    throw std::runtime_error(std::format("Could not parse mod URL: \"{}\".", mod_url));

  const auto [domain_name, mod_id] = *domain_and_mod;
  cpr::Response response =
    cpr::Get(cpr::Url(std::format(
               "https://api.nexusmods.com/v1/games/{}/mods/{}/files/{}/download_link.json",
               domain_name,
               mod_id,
               file_id)),
             cpr::Header{ { "apikey", api_key_ } });
  if(response.status_code == 403)
    throw std::runtime_error(
      "Generation of download links for NexusMods is restricted to premium accounts."
      "You can download the mod on the website here:\n" +
      std::format(
        "https://www.nexusmods.com/{}/mods/{}?tab=files&file_id={}", domain_name, mod_id, file_id));
  else if(response.status_code == 404)
    throw std::runtime_error("The requested file does not exist in NexusMods.");
  else if(response.status_code != 200)
    throw std::runtime_error(std::format("Failed to generate a download link for \"{}\"", mod_url));

  Json::Value json_body;
  Json::Reader reader;
  bool success = reader.parse(response.text.c_str(), json_body);
  if(!success)
    throw ParseError("Failed to parse response from NexusMods.");

  return json_body[0]["URI"].asString();
}

std::string Api::getDownloadUrl(const std::string& nxm_url)
{
  const auto match_opt = nxmUrlIsValid(nxm_url);
  if(!match_opt)
    throw std::runtime_error(std::format("Invalid NXM URL: \"{}\"", nxm_url));
  std::smatch match = *match_opt;
  const std::string domain_name = match[1];
  const std::string mod_id = match[2];
  const std::string file_id = match[3];
  const std::string key = match[4];
  const std::string expires = match[5];

  cpr::Response response =
    cpr::Get(cpr::Url(std::format(
               "https://api.nexusmods.com/v1/games/{}/mods/{}/files/{}/download_link.json",
               domain_name,
               mod_id,
               file_id)),
             cpr::Header{ { "apikey", api_key_ } },
             cpr::Parameters{ { "game_domain_name", domain_name },
                              { "id", file_id },
                              { "mod_id", mod_id },
                              { "key", key },
                              { "expires", expires } });
  if(response.status_code == 400)
    throw std::runtime_error("Failed to generate download link. Check if the account used on "
                             "NexusMods matches the one for the API key in Limo.");
  else if(response.status_code == 404)
    throw std::runtime_error(std::format("File with id {} for mod with id {} for application"
                                         "\"{}\" not found on NexusMods.",
                                         file_id,
                                         mod_id,
                                         domain_name));
  else if(response.status_code == 410)
    throw std::runtime_error("The NexusMods download link has expired.");
  else if(response.status_code != 200)
    throw std::runtime_error(std::format("Failed to generate download link for file with id {} "
                                         "for mod with id {} for application {}.",
                                         file_id,
                                         mod_id,
                                         domain_name));

  Json::Value json_body;
  Json::Reader reader;
  bool success = reader.parse(response.text.c_str(), json_body);
  if(!success)
    throw ParseError("Failed to parse response from NexusMods.");

  return json_body[0]["URI"].asString();
}

std::vector<std::pair<std::string, std::vector<std::string>>> Api::getChangelogs(
  const std::string& mod_url)
{
  std::vector<std::pair<std::string, std::vector<std::string>>> changelogs;
  auto domain_and_mod = extractDomainAndModId(mod_url);
  if(!domain_and_mod)
    throw std::runtime_error(std::format("Could not parse mod URL: \"{}\".", mod_url));

  const auto [domain_name, mod_id] = *domain_and_mod;
  cpr::Response response = cpr::Get(
    cpr::Url(std::format(
      "https://api.nexusmods.com/v1/games/{}/mods/{}/changelogs.json", domain_name, mod_id)),
    cpr::Header{ { "apikey", api_key_ } });
  if(response.status_code != 200)
    throw std::runtime_error(std::format(
      "Failed to get changelogs for mod with id {} from NexusMods. Response code was {}",
      mod_id,
      response.status_code));

  Json::Value json_body;
  Json::Reader reader;
  bool success = reader.parse(response.text.c_str(), json_body);
  if(!success)
    throw ParseError("Failed to parse response from NexusMods.");

  std::string text = response.text;
  if(text.starts_with('\"'))
    text.erase(0, 1);
  if(text.ends_with('\"'))
    text.erase(text.size() - 1, 1);

  for(const auto& key : json_body.getMemberNames())
  {
    std::vector<std::string> changes;
    auto log = json_body[key];
    for(int i = 0; i < log.size(); i++)
      changes.push_back(log[i].asString());
    changelogs.emplace_back(key, changes);
  }
  // Jsoncpp uses a std::map to store key, value pairs. This messes up the order of the keys, so
  // they have be re-sorted by version number
  std::sort(changelogs.begin(),
            changelogs.end(),
            [](auto a, auto b)
            {
              std::regex regex(R"(.*?(\d+)\.?(.*))");
              std::smatch match;
              std::vector<int> a_parts;
              std::vector<int> b_parts;
              std::string target = a.first;
              bool found = false;
              while(std::regex_search(target, match, regex))
              {
                found = true;
                a_parts.push_back(std::stoi(match[1]));
                target = match[2];
              }
              if(!found)
                return a > b;

              found = false;
              target = b.first;
              while(std::regex_search(target, match, regex))
              {
                found = true;
                b_parts.push_back(std::stoi(match[1]));
                target = match[2];
              }
              if(!found)
                return a > b;

              for(auto [a_num, b_num] : str::zip_view(a_parts, b_parts))
              {
                if(a_num != b_num)
                  return a_num > b_num;
              }
              return a > b;
            });
  return changelogs;
}

bool Api::modUrlIsValid(const std::string& url)
{
  if(url.empty())
    return false;
  const std::regex regex(R"((?:https:\/\/)?www\.nexusmods\.com\/(.+)\/mods\/(\d+).*)");
  return std::regex_match(url, regex);
}

Page Api::getNexusPage(const std::string& mod_url)
{
  return { mod_url, getMod(mod_url), getChangelogs(mod_url), getModFiles(mod_url) };
}

std::optional<std::pair<std::string, bool>> Api::validateKey(const std::string& api_key)
{
  cpr::Response response = cpr::Get(cpr::Url("https://api.nexusmods.com/v1/users/validate.json"),
                                    cpr::Header{ { "apikey", api_key } });
  if(response.status_code != 200)
    return {};

  Json::Value json_body;
  Json::Reader reader;
  bool success = reader.parse(response.text.c_str(), json_body);
  if(!success)
    throw ParseError("Failed to parse response from NexusMods.");

  return { { json_body["name"].asString(), json_body["is_premium"].asBool() } };
}

std::string Api::getNexusPageUrl(const std::string& nxm_url)
{
  std::regex nxm_regex(R"(nxm:\/\/(.*)\/mods\/(\d+)\/files\/\d+\?.*)");
  std::smatch match;
  if(!std::regex_match(nxm_url, match, nxm_regex))
    throw std::runtime_error("Invalid nxm url: \"" + nxm_url + "\".");
  return std::format("https://www.nexusmods.com/{}/mods/{}", match[1].str(), match[2].str());
}

std::string Api::getApiKey()
{
  return api_key_;
}

std::optional<std::pair<std::string, int>> Api::extractDomainAndModId(const std::string& mod_url)
{
  const std::regex regex(R"((?:https:\/\/)?www\.nexusmods\.com\/(.+)\/mods\/(\d+).*)");
  std::smatch match;
  if(std::regex_match(mod_url, match, regex))
    return { { match[1], std::stoi(match[2]) } };
  return {};
}

bool Api::initModInfo(ImportModInfo& info)
{
  std::vector<File> files;
  auto match = nxmUrlIsValid(info.remote_request_url);
  if(!match)
    return false;

  if(modUrlIsValid(info.remote_source))
    files = getModFiles(info.remote_source);
  else
  {
    info.remote_source = getNexusPageUrl(info.remote_request_url);
    files = getModFiles(info.remote_source);
  }

  const std::string file_id_str = (*match)[3];
  if(file_id_str.find_first_not_of("0123456789") != std::string::npos)
    return false;

  const std::string mod_id_str = (*match)[2];
  if(mod_id_str.find_first_not_of("0123456789") != std::string::npos)
    return false;

  long file_id = std::stol(file_id_str);
  long mod_id = std::stol(mod_id_str);
  auto iter = str::find_if(files, [file_id](File& f){return f.file_id == file_id;});
  if(iter == files.end())
    return false;

  info.remote_mod_id = mod_id;
  info.remote_file_id = iter->file_id;
  info.remote_file_name = iter->name;
  info.remote_file_version = iter->version;
  info.remote_type = ImportModInfo::RemoteType::nexus;

  return true;
}

std::optional<std::smatch> Api::nxmUrlIsValid(const std::string& nxm_url)
{
  const std::regex regex(
    R"(nxm:\/\/(.+)\/mods\/(\d+)\/files\/(\d+)\?key=(.+)&expires=(\d+)&user_id=(\d+))");
  std::smatch match;
  std::regex_match(nxm_url, match, regex);
  if(match.empty())
    return {};
  return match;
}
