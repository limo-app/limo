/*!
 * \file lspakheader.h
 * \brief Header for the LsPakHeader struct
 */

#pragma once

#include <cstdint>


/*!
 * \brief Represents the header of a .pak archive file used for Baldurs Gate 3.
 */
struct LsPakHeader
{
  /*! \brief Magic number indicating file type. */
  uint32_t magic_number;
  /*! \brief Archive format version. */
  uint32_t version;
  /*! \brief Offset of a list of compressed files in the archive. */
  uint64_t file_list_offset;
  /*! \brief Size of the compressed file list. */
  uint32_t file_list_size;
  /*! \brief Contains various flags. */
  uint8_t flags;
  /*! \brief Priority of the file. */
  uint8_t priority;
  /*! \brief MD5 hash of the file. */
  char md5[16];
  /*! \brief Number of separate archives within the file. */
  uint16_t num_parts;
} __attribute__((packed));
