/*!
 * \file installstep.h
 * \brief Header for the InstallStep struct.
 */

#pragma once

#include "dependency.h"
#include "plugingroup.h"
#include <vector>


/*!
 * \brief The fomod namespace contains classes used for parsing a FOMOD xml file and for
 * creating an installer.
 */
namespace fomod
{
/*! \brief A step during installation. */
struct InstallStep
{
  /*! \brief Step name. */
  std::string name;
  /*! \brief Step description. */
  Dependency dependencies;
  /*! \brief Sets of choices displayed during this step. */
  std::vector<PluginGroup> groups;
};
}
