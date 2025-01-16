/*!
 * \file lspakfilelistentry.h
 * \brief Header for the LsPakFileListEntry class
 */

#pragma once

#include <cstdint>


/*!
 * \brief Represents an entry in the file list of a .pak archive used for Baldurs Gate 3.
 */
struct LsPakFileListEntry
{
  /*! \brief Path to which to extract the file. */
  char path[256];
  /*! \brief Offset of the compressed file in the archive. */
  uint64_t offset : 48;
  /*! \brief Archive containing the file. */
  uint8_t archive_part;
  /*! \brief Flags indicating compression type. */
  uint8_t flags;
  /*! \brief Compressed size of the file. */
  uint32_t compressed_size;
  /*! \brief Uncompressed size of the file. */
  uint32_t uncompressed_size;
} __attribute__((packed));
