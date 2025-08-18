#include "test_utils.h"
#include <fstream>

std::vector<std::string> getFiles(sfs::path dir, bool get_contents = false)
{
  std::vector<std::string> files;
  for(const auto& dir_entry : sfs::recursive_directory_iterator(dir))
  {
    if(dir_entry.path().filename() == ".lmmfiles" || dir_entry.path().filename() == ".lmm_managed_dir")
      continue;
    std::string entry = dir_entry.path().string().erase(0, dir.string().size());
    if(get_contents && dir_entry.is_regular_file())
    {
      std::ifstream file(dir_entry.path());
      entry.append(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
      file.close();
    }
    files.push_back(entry);
  }
  return files;
}

void verifyDirsAreEqual(sfs::path first_dir, sfs::path second_dir, bool test_content)
{
  std::vector<std::string> first_dir_files = getFiles(first_dir, test_content);
  std::vector<std::string> second_dir_files = getFiles(second_dir, test_content);

  CAPTURE(first_dir.string());
  CAPTURE(second_dir.string());
  REQUIRE(second_dir_files.size() == first_dir_files.size());
  REQUIRE_THAT(first_dir_files, Catch::Matchers::UnorderedEquals(second_dir_files));
}

void resetAppDir()
{
  if(sfs::exists(DATA_DIR / "app"))
    sfs::remove_all(DATA_DIR / "app");
  sfs::copy_options options(sfs::copy_options::recursive | sfs::copy_options::overwrite_existing);
  sfs::copy(DATA_DIR / "source" / "app", DATA_DIR / "app", options);
}

void resetStagingDir()
{
  if(sfs::exists(DATA_DIR / "staging"))
    sfs::remove_all(DATA_DIR / "staging");
  sfs::create_directories(DATA_DIR / "staging");
}

void verifyFilesAreEqual(sfs::path first_file, sfs::path second_file)
{
  CAPTURE(first_file.string());
  CAPTURE(second_file.string());
  REQUIRE(sfs::exists(first_file));
  REQUIRE(sfs::exists(second_file));
  std::string content_first_file = "";
  std::ifstream file(first_file);
  content_first_file.append(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  file.close();
  std::string content_second_file = "";
  file.open(second_file);
  content_second_file.append(std::istreambuf_iterator<char>(file),
                             std::istreambuf_iterator<char>());
  file.close();
  REQUIRE(content_first_file == content_second_file);
}

