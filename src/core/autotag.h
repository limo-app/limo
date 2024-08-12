/*!
 * \file autotag.h
 * \brief Header for the AutoTag class.
 */

#pragma once

#include "pathutils.h"
#include "progressnode.h"
#include "tag.h"
#include "tagconditionnode.h"
#include <filesystem>
#include <json/json.h>
#include <optional>
#include <string>
#include <vector>


/*!
 * \brief Tag which is automatically added to a mod when its files fulfill the tags conditions.
 * Conditions are managed by a TagConditionNode object.
 */
class AutoTag : public Tag
{
public:
  /*!
   * \brief Constructor.
   * \param name Name of the new tag.
   * \param expression Boolean expression used to combine the given conditions. The tag is applied
   * to a mod when this evaluates to true.
   * \param conditions Vector of conditions used to decide if this tag is to be applied. These
   * act as variables in the tags expression.
   */
  AutoTag(const std::string& name,
          const std::string& expression,
          const std::vector<TagCondition>& conditions);
  /*!
   * \brief Deserializes an AutoTag from the given json object.
   * \param json Source json object.
   * \throws ParseError when the json object is invalid.
   */
  AutoTag(const Json::Value& json);

  /*!
   * \brief Removes this tag from all mods, then applies it to all given mods which
   * fulfill its conditions.
   * \param files Maps mod ids to a vector of pairs of paths and file names for that mod.
   * \param mods Iterable container containing int ids of all mods to be checked.
   * \param progress_node Used to inform about progress.
   */
  template<typename View>
  void reapplyMods(const std::map<int, std::vector<std::pair<std::string, std::string>>>& files,
                   const View& mods,
                   std::optional<ProgressNode*> progress_node = {})
  {
    mods_.clear();
    for(int mod : mods)
    {
      if(evaluator_.evaluate(files.at(mod)))
        mods_.push_back(mod);
      if(progress_node)
        (*progress_node)->advance();
    }
  }
  /*!
   * \brief Removes this tag from all mods, then applies it to all given mods which
   * fulfill its conditions.
   * \param staging_dir Directory containing the mods.
   * \param mods Iterable container containing int ids of all mods to be checked.
   * \param progress_node Used to inform about progress.
   */
  template<typename View>
  void reapplyMods(const std::filesystem::path& staging_dir,
                   const View& mods,
                   std::optional<ProgressNode*> progress_node = {})
  {
    reapplyMods(readModFiles(staging_dir, mods), mods, progress_node);
  }
  /*!
   * \brief Reevaluates if the given mods should have this tag. Adds/ removes the tag
   * from all given mods when needed.
   * \param files Maps mod ids to a vector of pairs of paths and file names for that mod.
   * \param mods Iterable container containing int ids of all mods to be checked.
   * \param progress_node Used to inform about progress.
   */
  template<typename View>
  void updateMods(const std::map<int, std::vector<std::pair<std::string, std::string>>>& files,
                  const View& mods,
                  std::optional<ProgressNode*> progress_node = {})
  {
    for(int mod : mods)
    {
      auto iter = std::ranges::find(mods_, mod);
      if(iter != mods_.end())
        mods_.erase(iter);
      if(evaluator_.evaluate(files.at(mod)))
        mods_.push_back(mod);
      if(progress_node)
        (*progress_node)->advance();
    }
  }
  /*!
   * \brief Reevaluates if the given mods should have this tag. Adds/ removes the tag
   * from all given mods when needed.
   * \param staging_dir Directory containing the mods.
   * \param mods Iterable container containing int ids of all mods to be checked.
   * \param progress_node Used to inform about progress.
   */
  template<typename View>
  void updateMods(const std::filesystem::path& staging_dir,
                  const View& mods,
                  std::optional<ProgressNode*> progress_node = {})
  {
    updateMods(readModFiles(staging_dir, mods), mods, progress_node);
  }
  /*!
   * \brief Changes the conditions and expression used by this tag.
   * \param expression The new expression.
   * \param conditions The new conditions.
   */
  void setEvaluator(const std::string& expression, const std::vector<TagCondition>& conditions);
  /*!
   * \brief Serializes this tag to a json object.
   * \return The json object.
   */
  Json::Value toJson() const;
  /*!
   * \brief Compares this tag by name to the given name.
   * \param name Name to compare to.
   * \return True if the names are identical.
   */
  bool operator==(const std::string& name) const;
  /*!
   * \brief Getter for this tags expression.
   * \return The expression.
   */
  std::string getExpression() const;
  /*!
   * \brief Getter for this tags conditions.
   * \return The conditions.
   */
  std::vector<TagCondition> getConditions() const;
  /*!
   * \brief Returns the number of conditions for this tag.
   * \return The number of conditions.
   */
  int getNumConditions() const;
  /*!
   * \brief Recursively iterates over all files for all mods with given ids and creates a
   * a map of mod ids to a vector containing pairs of path and file name.
   * This vector is used as input for the reapplyMods and updateMods functions.
   * \param staging_dir Staging directory for the given mods.
   * \param mods Iterable container containing int ids of all mods to be checked.
   * \param progress_node Used to inform about progress.
   * \return The map.
   */
  template<typename View>
  static std::map<int, std::vector<std::pair<std::string, std::string>>> readModFiles(
    const std::filesystem::path& staging_dir,
    View mods,
    std::optional<ProgressNode*> progress_node = {})
  {
    std::map<int, std::vector<std::pair<std::string, std::string>>> files;
    for(int mod : mods)
    {
      files[mod] = {};
      const std::filesystem::path mod_path = staging_dir / std::to_string(mod);
      for(const auto& dir_entry : std::filesystem::recursive_directory_iterator(mod_path))
      {
        std::string path = path_utils::getRelativePath(dir_entry.path(), mod_path);
        if(path.front() == '/')
          path.erase(0, 1);
        files[mod].emplace_back(path, dir_entry.path().filename().string());
      }
      if(progress_node)
        (*progress_node)->advance();
    }
    return files;
  }

private:
  /*! \brief Expression used by the TagConditionNode. */
  std::string expression_;
  /*! \brief Conditions used by the TagConditionNode. */
  std::vector<TagCondition> conditions_;
  /*!
   * \brief This tag is applied to a mod if this nodes evaluate function returns true for
   *  the mods installation directory
   */
  TagConditionNode evaluator_;
};
