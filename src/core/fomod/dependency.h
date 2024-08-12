/*!
 * \file fomoddependency.h
 * \brief Header for the FomodDependency class and FomodFile struct.
 */

#pragma once

#include "pugixml.hpp"
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>


/*!
 * \brief The fomod namespace contains classes used for parsing a FOMOD xml file and for
 * creating an installer.
 */
namespace fomod
{
/*!
 * \brief Represents a fomod dependency tree node.
 */
class Dependency
{
  /*! \brief Represents different dependency types. */
  enum Type
  {
    /*! \brief Always evaluates to true. */
    dummy_node,
    /*! \brief True if all children evaluate to true. */
    and_node,
    /*! \brief True if at least one child evaluates to true. */
    or_node,
    /*! \brief File must exist. */
    file_leaf,
    /*! \brief Flag must be set. */
    flag_leaf,
    /*! \brief Game version must be == some version. */
    game_version_leaf,
    /*! \brief Fomm version must be == some version. */
    fomm_version_leaf
  };


public:
  /*!
   * \brief Recursively builds a dependency tree from given fomod node.
   * \param source Source fomod node.
   */
  Dependency(pugi::xml_node source);
  /*! \brief Constructs a dummy node. */
  Dependency();

  /*!
   * \brief Checks given flags, files, game version and fomm version fulfill the condition
   * represented by this tree.
   * \param target_path Path to target files.
   * \param flags Flags to be checked.
   * \param eval_game_version Used to check if this nodes game version is valid.
   * \param eval_fomm_version Used to check if this nodes fomm version is valid.
   * \return True if conditions are met, else false.
   */
  bool evaluate(
    const std::filesystem::path& target_path,
    const std::map<std::string, std::string>& flags,
    std::function<bool(std::string)> eval_game_version,
    std::function<bool(std::string)> eval_fomm_version = [](auto s) { return true; }) const;
  std::string toString() const;

private:
  /*! \brief Type of this dependency. */
  Type type_;
  /*! \brief Value for comparison, e.g. file path for a file dependency. */
  std::string target_;
  /*! \brief State of file or flag. */
  std::string state_;
  /*! \brief Children of this node. */
  std::vector<Dependency> children_;
};
}
