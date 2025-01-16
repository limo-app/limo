#include "lspakextractor.h"
#include <format>
#include <fstream>
#include <iostream>
#include <lz4.h>
#include <vector>
#include <zstd.h>
#include <zlib.h>

namespace sfs = std::filesystem;


LsPakExtractor::LsPakExtractor(const sfs::path& source_path) : source_path_(source_path) {}

void LsPakExtractor::init()
{
  std::ifstream file(source_path_, std::ios::binary);
  header_ = std::make_unique<LsPakHeader>();
  file.read(reinterpret_cast<char*>(header_.get()), sizeof(LsPakHeader));

  if(static_cast<unsigned int>(header_->magic_number) != LS_PAK_MAGIC_HEADER_NUMBER)
    throw std::runtime_error(std::format("Unknown file format with magic number: {}",
                                         static_cast<unsigned int>(header_->magic_number)));
  if(static_cast<unsigned int>(header_->version) != LS_PAK_SUPPORTED_VERSION)
  {
    throw std::runtime_error(
      std::format("Unsupported file version: {}", static_cast<unsigned int>(header_->version)));
  }

  auto compressed_size = readFileList();
  if(compressed_size + 8 != header_->file_list_size)
  {
    throw std::runtime_error(std::format("Mismatch for file list size! Expected {}, found {}.",
                                         static_cast<unsigned int>(header_->file_list_size - 8),
                                         compressed_size));
  }
}

std::string LsPakExtractor::extractData(unsigned long offset,
                                        unsigned int length,
                                        unsigned int uncompressed_size,
                                        int compression_type)
{
  // this is used to extract xml files; they should never exceed 1GiB
  if(uncompressed_size > 1 << 30)
    throw std::runtime_error(std::format("Uncompressed file size is too large: {}B.", uncompressed_size));

  std::ifstream file(source_path_, std::ios::binary);
  std::vector<char> input_buffer(length);
  file.seekg(offset);
  file.read(input_buffer.data(), length);

  if(compression_type == COMPRESSION_NONE)
    return { input_buffer.data(), length };
  else if(compression_type == COMPRESSION_LZ4)
  {
    std::vector<char> output_buffer(uncompressed_size);
    int ret_code = LZ4_decompress_safe_partial(
      input_buffer.data(), output_buffer.data(), length, uncompressed_size, uncompressed_size);
    if(ret_code < 0)
      throw std::runtime_error(std::format("LZ4 decompression failed with code: {}", ret_code));

    return { output_buffer.data(), uncompressed_size };
  }
  else if(compression_type == COMPRESSION_ZSTD)
  {
    std::vector<char> output_buffer(uncompressed_size);
    const size_t actual_size = ZSTD_decompress(reinterpret_cast<void*>(output_buffer.data()),
                                               uncompressed_size,
                                               reinterpret_cast<const void*>(input_buffer.data()),
                                               input_buffer.size());
    if(ZSTD_isError(actual_size))
      throw std::runtime_error(std::format("zstd decompression failed with code: {}", actual_size));
    return { output_buffer.data(), uncompressed_size };
  }
  else if(compression_type == COMPRESSION_ZLIB)
  {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    if (inflateInit(&stream) != Z_OK)
      throw std::runtime_error("zlib initialization failed.");
    stream.avail_in = input_buffer.size();
    stream.next_in = reinterpret_cast<Bytef*>(input_buffer.data());

    std::vector<char> output_buffer(uncompressed_size);
    stream.avail_out = uncompressed_size;
    stream.next_out = reinterpret_cast<Bytef*>(output_buffer.data());
    inflateInit(&stream);
    int code = inflate(&stream, Z_NO_FLUSH);
    inflateEnd(&stream);
    if(code < 0)
      throw std::runtime_error(std::format("zlib decompression failed with code: {}", code));
    return { output_buffer.data(), uncompressed_size };
  }
  else
    throw std::runtime_error(std::format("Unsopported compression type: {}", compression_type));
}

std::vector<std::filesystem::path> LsPakExtractor::getFileList()
{
  std::vector<sfs::path> path_list;
  for(const auto& f : file_list_)
    path_list.emplace_back(f.path);
  return path_list;
}

std::string LsPakExtractor::extractFile(int file_id)
{
  const auto& file = file_list_[file_id];
  return extractData(
    file.offset, file.compressed_size, file.uncompressed_size, file.flags & COMPRESSION_MASK);
}

unsigned int LsPakExtractor::readFileList()
{
  std::ifstream file(source_path_, std::ios::binary);
  file.seekg(header_->file_list_offset);
  std::vector<char> buffer(4);
  file.read(buffer.data(), 4);
  unsigned int num_files = *reinterpret_cast<unsigned int*>(buffer.data());
  file.read(buffer.data(), 4);
  unsigned int compressed_size = *reinterpret_cast<unsigned int*>(buffer.data());
  std::string data = extractData(
    file.tellg(), compressed_size, sizeof(LsPakFileListEntry) * num_files, COMPRESSION_LZ4);

  file_list_.clear();
  for(int i = 0; i < sizeof(LsPakFileListEntry) * num_files; i += sizeof(LsPakFileListEntry))
    file_list_.push_back(*reinterpret_cast<LsPakFileListEntry*>(data.data() + i));
  return compressed_size;
}
