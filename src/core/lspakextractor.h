/*!
 * \file lspakextractor.h
 * \brief Header for the LsPakExtractor class
 */

#pragma once

#include "lspakfilelistentry.h"
#include "lspakheader.h"
#include <filesystem>
#include <vector>


/*!
 * \brief Class providing functions for extracting files from a .pak archive used for Baldurs Gate 3.
 */
class LsPakExtractor
{
public:
  /*!
   * \brief Sets the archive path to the given path.
   * \param source_path Target archive path.
   */
  LsPakExtractor(const std::filesystem::path& source_path);

  /*! \brief Initializes header and file list from the source archive. */
  void init();
  /*!
   * \brief Returns a vector of paths to files in the archive.
   * \return The vector.
   */
  std::vector<std::filesystem::path> getFileList();
  /*!
   * \brief Decompresses a file in the archive.
   * \param file_id Index of the file to the extracted.
   * \return The uncompressed file as a string.
   */
  std::string extractFile(int file_id);

private:
  /*! \brief Mask used to get compression type from file list entry flags. */
  static constexpr int COMPRESSION_MASK = 0xf;
  /*! \brief Indicates file is uncompressed. */
  static constexpr int COMPRESSION_NONE = 0;
  /*! \brief Indicates file is compressed using zlib. */
  static constexpr int COMPRESSION_ZLIB = 1;
  /*! \brief Indicates file is compressed using lz4. */
  static constexpr int COMPRESSION_LZ4 = 2;
  /*! \brief Indicates file is compressed using zstd. */
  static constexpr int COMPRESSION_ZSTD = 3;
  /*! \brief Indicates file is a supported .pak archive. */
  static constexpr unsigned int LS_PAK_MAGIC_HEADER_NUMBER = 0x4b50534c;
  /*! \brief Currently the only supported archive format version. */
  static constexpr unsigned int LS_PAK_SUPPORTED_VERSION = 18;
  /*! \brief Path to the source archive. */
  std::filesystem::path source_path_;
  /*! \brief Contains the archive's header. */
  std::unique_ptr<LsPakHeader> header_;
  /*! \brief Contains all file list entries of the archive. */
  std::vector<LsPakFileListEntry> file_list_;

  /*!
   * \brief Extracts and, if necessary, decompresses data from the source archive.
   * \param offset Offset from which to extract data.
   * \param length Number of bytes to read.
   * \param uncompressed_size Uncompressed size of the data.
   * \param compression_type Compression type used.
   * \return The uncompressed data as a string.
   */
  std::string extractData(unsigned long offset,
                          unsigned int length,
                          unsigned int uncompressed_size,
                          int compression_type);
  /*!
   * \brief Reads the file list from the source archive and initializes file_list_.
   * \return The compressed size of the file list.
   */
  unsigned int readFileList();
};
