/*!
 * \file plugingroup.h
 * \brief Header for the PluginGroup struct.
 */

#pragma once

#include "plugin.h"
#include <string>
#include <vector>


/*!
 * \brief The fomod namespace contains classes used for parsing a FOMOD xml file and for
 * creating an installer.
 */
namespace fomod
{
/*! \brief Represents a set of options which can be selected during installation. */
struct PluginGroup
{
  /*! \brief Describes restriction on how plugins in a group can be selected. */
  enum Type
  {
    /*! \brief At least one plugin must be selected. */
    at_least_one,
    /*! \brief At most one plugin must be selected. */
    at_most_one,
    /*! \brief Exactly one plugin must be selected. */
    exactly_one,
    /*! \brief All plugins must be selected. */
    all,
    /*! \brief No restrictions on selection. */
    any
  };

  /*! \brief Group name. */
  std::string name;
  /*! \brief Selection restrictions. */
  Type type;
  /*! \brief Selectable plugins in this group. */
  std::vector<Plugin> plugins;
};
}
