/*!
 * \file pathutils.h
 * \brief Header for the path_utils namespace.
 */

#pragma once

#include <filesystem>
#include <functional>
#include <optional>


/*!
 * \brief Contains utility functions for dealing with std::filesystem::path objects.
 */
namespace path_utils
{
/*!
 * \brief Checks if the target path exists.
 * \param target Path to check.
 * \param base_path If specified, target path is appended to this path during the search.
 * \param case_insensitive If true: Ignore case mismatch for path search.
 * \return The target path in its actual case, if found.
 */
std::optional<std::filesystem::path> pathExists(const std::filesystem::path& path_to_check,
                                                const std::filesystem::path& base_path,
                                                bool case_insensitive = true);

/*!
 * \brief Returns a string containing the given path in lower case.
 * \param path Path to be converted.
 * \return The lower case path.
 */
std::string toLowerCase(const std::filesystem::path& path);
/*!
 * \brief Recursively moves all files from the source directory to the target directory.
 * \param source Source directory.
 * \param destination Target directory.
 * \param move If false: Copy files instead of moving them.
 */
void moveFilesToDirectory(const std::filesystem::path& source,
                          const std::filesystem::path& destination,
                          bool move = true);
/*!
 * \brief Replaces all double backslash path separators with a forward slash.
 * \return The normalized path.
 */
std::string normalizePath(const std::string& path);
/*!
 * \brief Determines the relative path from source to target. Only works if source.string()
 * is a sub-string of target.string().
 * \param target Target path.
 * \param source Source path.
 * \return The relative path.
 */
std::string getRelativePath(std::filesystem::path target, std::filesystem::path source);
/*!
 * \brief Returns true if directory is empty or contains only empty directories.
 * \param directory Directory to check.
 * \return True if empty, else false.
 */
bool directoryIsEmpty(const std::filesystem::path& directory);
/*!
 * \brief Returns the number of elements in given path.
 * \param path Path to be checked.
 * \return The length.
 */
int getPathLength(const std::filesystem::path& path);
/*!
 * \brief Removes the first components of a given path.
 * \param path Source path.
 * \param depth Components with depth < this will be removed.
 * \return A pair of the removed components and the shortened path.
 */
std::pair<std::filesystem::path, std::filesystem::path> removePathComponents(
  const std::filesystem::path& path,
  int depth);
/*!
 * \brief Recursively renames all files at given source directory using given converter,
 * then copies the result to given destination directory.
 * \param destination Path to destination directory for renamed files.
 * \param source Path to source files to be renamed.
 * \param converter Function which converts one char to another, e.g. converting to
 * upper case.
 */
void renameFiles(const std::filesystem::path& destination,
                 const std::filesystem::path& source,
                 std::function<unsigned char(unsigned char)> converter);
/*!
 * \brief Recursively moves all files from source to destination, removes all
 * path components with depth < root_level.
 * \param source Source path.
 * \param destination Destination path.
 * \param depth Minimum depth for path components to keep.
 */
void moveFilesWithDepth(const std::filesystem::path& source,
                        const std::filesystem::path& destination,
                        int depth);

/*!
 * \brief Copies or moves files from source to dest.
 * \param source Copy/ move source path.
 * \param destination Copy/ move target path.
 * \param move If true: Move files, else: Recursively copy files.
 */
void copyOrMoveFiles(const std::filesystem::path& source,
                     const std::filesystem::path& destination,
                     bool move);

}
