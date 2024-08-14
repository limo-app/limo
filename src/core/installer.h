/*!
 * \file installer.h
 * \brief Header for the Installer class
 */

#pragma once

#include "progressnode.h"
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <vector>


/*!
 * \brief Holds static functions to install and uninstall mods.
 */
class Installer
{
public:
  /*! \brief Flags used for installation options. */
  enum Flag
  {
    preserve_case = 0,
    lower_case = 1 << 0,
    upper_case = 1 << 1,
    preserve_directories = 1 << 2,
    single_directory = 1 << 3
  };
  /*! \brief Every vector represents an exclusive group of flags. */
  inline static const std::vector<std::vector<Flag>> OPTION_GROUPS{
    { preserve_case, lower_case, upper_case },
    { preserve_directories, single_directory }
  };
  /*! \brief Maps installer flags to descriptive names. */
  inline static const std::map<Flag, std::string> OPTION_NAMES{
    { preserve_case, "Preserve file names" },
    { lower_case, "Convert to lower case" },
    { upper_case, "Convert to upper case" },
    { preserve_directories, "Preserve directories" },
    { single_directory, "Root directory only" }
  };
  /*! \brief Maps installer flags to brief descriptions of what they do. */
  inline static const std::map<Flag, std::string> OPTION_DESCRIPTIONS{
    { preserve_case, "Do not alter file names" },
    { lower_case, "Convert file and directory names to lower case (FiLe -> file)" },
    { upper_case, "Convert file and directory names to upper case (FiLe -> FILE)" },
    { preserve_directories, "Do not alter directory structure" },
    { single_directory, "Move files from all sub directories to the mods root directory" }
  };
  /*! \brief Simply extracts files */
  inline static const std::string SIMPLEINSTALLER{ "Simple Installer" };
  /*!
   * \brief Takes a vector of files created by fomod::FomodInstaller and
   * moves them to their target.
   */
  inline static const std::string FOMODINSTALLER{ "Fomod Installer" };
  /*!
   * \brief Contains all available installer types.
   */
  inline static const std::vector<std::string> INSTALLER_TYPES{ SIMPLEINSTALLER, FOMODINSTALLER };

  /*!
   * \brief Extracts the given archive to the given directory.
   * \param source Path to the archive.
   * \param destination Destination directory for extraction.
   * \param progress_node Used to inform about extraction progress.
   * \return Int indicating success(0), a filesystem error(-2) or an error
   * during extraction(-1).
   */
  static void extract(const std::filesystem::path& source,
                      const std::filesystem::path& destination,
                      std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Extracts the archive, performs any actions specified by the installer type,
   * then copies all files to given destination.
   * \param path Path to the archive.
   * \param destination Destination directory for the installation.
   * \param options Sum of installation flags
   * \param installer Installer type to use.
   * \param root_level If > 0: Ignore all mod files and path components with depth <
   * root_level.
   * \return The total file size of the installed mod on disk.
   */
  static unsigned long install(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    int options,
    const std::string& type = SIMPLEINSTALLER,
    int root_level = 0,
    const std::vector<std::pair<std::filesystem::path, std::filesystem::path>> fomod_files = {});
  /*!
   * \brief Uninstalls the mod at given directory using the given installer type.
   * \param path Path to the mod.
   * \param installer Installer type to use.
   */
  static void uninstall(const std::filesystem::path& mod_path,
                        const std::string& type = SIMPLEINSTALLER);
  /*!
   * \brief Recursively reads all file and directory names from given archive.
   * \param path Path to given archive.
   * \return Vector of paths within the archive.
   */
  static std::vector<std::filesystem::path> getArchiveFileNames(const std::filesystem::path& path);
  /*!
   * \brief Identifies the appropriate installer type from given source archive or
   * directory.
   * \param source Path to mod source.
   * \return Required root level and type of the installer.
   */
  static std::tuple<int, std::string, std::string> detectInstallerSignature(
    const std::filesystem::path& source);
  /*!
   * \brief Deletes all temporary files created during a previous installation attempt.
   * \param staging_dir Directory containing temporary files.
   * \param mod_id Id of the mod whose installation failed.
   */
  static void cleanupFailedInstallation(const std::filesystem::path& staging_dir, int mod_id);
  /*!
   * \brief Sets whether this application is running as a flatpak.
   * \param is_a_flatpak If true: The application is running as a flatpak.
   */
  static void setIsAFlatpak(bool is_a_flatpak);

private:
  /*! \brief Directory name used to temporary storage of files during installation. */
  static inline std::string EXTRACT_TMP_DIR = "lmm_tmp_extract";
  /*! \brief Extension used for temporary storage during file movement. */
  static inline std::string MOVE_EXTENSION = "tmpmove";
  /*! \brief If true: The application is running as a flatpak. */
  static inline bool is_a_flatpak_ = false;

  /*!
   * \brief Throws a CompressionError containing the error message of given archive.
   * \param source Archive containing the error message.
   */
  static void throwCompressionError(struct archive* source);
  /*!
   * \brief Copies data from given source archive to given destination archive.
   * Throws CompressionError when an reading or writing fails.
   * \param source Source archive.
   * \param dest Destination archive.
   */
  static void copyArchive(struct archive* source, struct archive* dest);

  /*!
   * \brief Extracts the given archive to the given directory. Informs about
   * extraction progress using the provided node.
   * \param source_path Path to the archive.
   * \param dest_path Destination directory for extraction.
   * \param progress_node Used to inform about extraction progress.
   */
  static void extractWithProgress(const std::filesystem::path& source_path,
                                  const std::filesystem::path& dest_path,
                                  std::optional<ProgressNode*> progress_node = {});
  /*!
   * \brief Libarchive sometime fails to extract certain rar archives when
   * using the method implemented in \ref extractWithProgress. This function
   * uses libunrar instead of libarchive to extract a given rar archive.
   * \param source_path Path to the archive.
   * \param dest_path Destination directory for extraction.
   */
  static void extractRarArchive(const std::filesystem::path& source_path,
                                const std::filesystem::path& dest_path);
};
