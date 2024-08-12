/*!
 * \file file.h
 * \brief Header for the File struct.
 */

#pragma once

#include <filesystem>


/*!
 * \brief The fomod namespace contains classes used for parsing a FOMOD xml file and for
 * creating an installer.
 */
namespace fomod
{
/*!
 * \brief Holds data regarding the installation of a single file in a fomod configuration.
 */
struct File
{
  /*! \brief Source path, relative to mods root directory. */
  std::filesystem::path source;
  /*! \brief Destination path, relative to target root.*/
  std::filesystem::path destination = "";
  /*! \brief If True: Always install, regardless of selection. */
  bool always_install = false;
  /*! \brief If True: Always install if dependencies are fulfilled. */
  bool install_if_usable = false;
  /*! \brief If two files share a destination, the higher priority file gets installed. */
  int priority = -std::numeric_limits<int>::max();

  /*!
   * \brief Compares two File objects by their destination.
   * \param other Other File.
   * \return True if destinations are equal.
   */
  bool operator==(const File& other) const
  {
    return destination.string() == other.destination.string();
  }
  /*!
   * \brief Compares two File objects by their priority.
   * \param other Other File.
   * \return True if this has lower priority.
   */
  bool operator<(const File& other) const { return priority < other.priority; }
};
}
