/*!
 * \file plugintype.h
 * \brief Header for the PluginType enum.
 */

#pragma once

#include <string>
#include <vector>


/*!
 * \brief The fomod namespace contains classes used for parsing a FOMOD xml file and for
 * creating an installer.
 */
namespace fomod
{
/*! \brief Describes how a plugin is presented. */
enum PluginType
{
  /*! \brief Always installed. */
  required,
  /*! \brief Can be installed. */
  optional,
  /*! \brief Should be installed. */
  recommended,
  /*! \brief Cannot be installed. */
  not_usable,
  /*! \brief Usage unclear, will be treated like optional. */
  could_be_usable
};

const std::vector<std::string> PLUGIN_TYPE_NAMES{ "Required",
                                                  "Optional",
                                                  "Recommended",
                                                  "Not Available",
                                                  "Could be usable" };
}
