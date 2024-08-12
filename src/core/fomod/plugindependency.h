/*!
 * \file plugindependency.h
 * \brief Header for the PluginDependency struct.
 */

#pragma once

#include "dependency.h"
#include "plugintype.h"


/*!
 * \brief The fomod namespace contains classes used for parsing a FOMOD xml file and for
 * creating an installer.
 */
namespace fomod
{
/*! \brief Represents a possible plugin type. */
struct PluginDependency
{
  /*! \brief Possible type. */
  PluginType type;
  /*! \brief Conditions which must be fulfilled for a plugin to take this type. */
  Dependency dependencies;
};
}
