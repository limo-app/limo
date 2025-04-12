/*!
 * \file Api.h
 * \brief Header for the nexus::Api class.
 */

#pragma once

#include "file.h"
#include "mod.h"
#include "../importmodinfo.h"
#include <cpr/cpr.h>
#include <string>
#include <regex>


/*!
 * \brief The nexus namespace contains structs and functions needed for accessing the NexusMods API.
 */
namespace nexus
{
/*!
 * \brief Contains all data for a mod available through the NexusMods api.
 */
struct Page
{
  /*! \brief URL of the mod page on NexusMods. */
  std::string url;
  /*! \brief Contains an overview of of the mod page, like a description and summary. */
  Mod mod;
  /*! \brief For every Version of the mod: A vector of changes in that version. */
  std::vector<std::pair<std::string, std::vector<std::string>>> changelog;
  /*! \brief Contains data on all available files for the mod. */
  std::vector<File> files;
};

/*!
 * \brief Provides functions for accessing the NexusMods API.
 */
class Api
{
public:
  /*! \brief This is an abstract class, so the constructor is deleted. */
  Api() = delete;

  /*!
   * \brief Sets the API key to use for all operations.
   * \param api_key The new API key.
   */
  static void setApiKey(const std::string& api_key);
  /*!
   * \brief Checks if this class has been initialized with an API key.
   * Does NOT check if the key works.
   * \return True if an API key exists.
   */
  static bool isInitialized();
  /*!
   * \brief Fetches data for the mod accessible by the given NexusMods URL.
   * \param mod_url URL to the mod on NexusMods.
   * \return A Mod object containing all received data.
   */
  static Mod getMod(const std::string& mod_url);
  /*!
   * \brief Fetches data for the mod specified by the NexusMods domain and mod id.
   * \param domain_name The NexusMods domain containing the mod.
   * \param mod_id Target mod id.
   * \return A Mod object containing all received data.
   */
  static Mod getMod(const std::string& domain_name, long mod_id);
  /*!
   * \brief Tracks the mod for the NexusMods account belonging to the API key.
   * \param mod_url URL to the mod on NexusMods.
   */
  static void trackMod(const std::string& mod_url);
  /*!
   * \brief Tracks the mod for the NexusMods account belonging to the API key.
   * \param mod_url URL to the mod on NexusMods.
   */
  static void untrackMod(const std::string& mod_url);
  /*!
   * \brief Fetches data for all mods tracked by the account belonging to the API key.
   * \return A vector of Mod objects with the received data.
   */
  static std::vector<Mod> getTrackedMods();
  /*!
   * \brief Fetches data for all available files for the given mod.
   * \param mod_url URL to the mod on NexusMods.
   * \return A vector of File objects containing the received data.
   */
  static std::vector<File> getModFiles(const std::string& mod_url);
  /*!
   * \brief Generates a download URL for the given mod file. This only works for premium accounts.
   * \param mod_url URL to the mod on NexusMods.
   * \param file_id Id of the file for which a link is to be generated.
   * \return The download URL.
   */
  static std::string getDownloadUrl(const std::string& mod_url, long file_id);
  /*!
   * \brief Generates a download URL from the given nxm Url.
   * \param nxm_url The nxm Url used. This is usually generated through the NexusMods website.
   * \return The download URL.
   */
  static std::string getDownloadUrl(const std::string& nxm_url);
  /*!
   * \brief Fetches changelogs for the given mod.
   * \param mod_url URL to the mod on NexusMods.
   * \return For every Version of the mod: A vector of changes in that version.
   */
  static std::vector<std::pair<std::string, std::vector<std::string>>> getChangelogs(
    const std::string& mod_url);
  /*!
   * \brief Checks if the given URL is a valid NexusMods mod page URL.
   * Only verifies if the URL is semantically correct, not if the target exists.
   * \param url URL to check.
   * \return True if the URL points to a NexusMods page.
   */
  static bool modUrlIsValid(const std::string& url);
  /*!
   * \brief Fetches data to fill a Page object for the given mod.
   * \param mod_url URL to the mod on NexusMods.
   * \return The generated Page object.
   */
  static Page getNexusPage(const std::string& mod_url);
  /*!
   * \brief Checks if the NexusMods API can be accessed with the given API key.
   * \param api_key API key to validate.
   * \return If the key works: The account name and a bool indicating if the account is premium.
   * Else: An empty std::optional.
   */
  static std::optional<std::pair<std::string, bool>> validateKey(const std::string& api_key);
  /*!
   * \brief Generates a NexusMods mod page URL from the given nxm URL.
   * \param nxm_url The nxm Url used. This is usually generated through the NexusMods website.
   * \return The NexusMods mod page URL.
   */
  static std::string getNexusPageUrl(const std::string& nxm_url);
  /*!
   * \brief Getter for the API key.
   * \return The API key.
   */
  static std::string getApiKey();
  /*!
   * \brief Extracts the NexusMods domain and mod id from the given mod page URL.
   * \param url URL to the mod on NexusMods.
   * \return If the given URL is valid: The domain and mod id. Else an empty std::optional.
   */
  static std::optional<std::pair<std::string, int>> extractDomainAndModId(
    const std::string& mod_url);
  /*!
   * \brief Initializes remote members of the given ImportModInfo.
   *
   * Uses data retreived for the mod associated with the ImportModInfo::remote_source member.
   * If remote_source is not valid, uses ImportModInfo::remote_download_url instead.
   *
   * \param info Mod info to initialize.
   * \return True if initialization was successful.
   */
  static bool initModInfo(ImportModInfo& info);
  /*!
   * \brief Checks whether the given string is a valid NexusMods nxm URL.
   * \param nxm_url String to check.
   * \return A regex match object for the string containing a group for every datum in the
   * URL. If the URL is invalid: An empty optional.
   */
  static std::optional<std::smatch> nxmUrlIsValid(const std::string& nxm_url);

private:
  /*! \brief The API key used for all operations. */
  inline static std::string api_key_ = "";
};
}
