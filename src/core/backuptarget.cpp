#include "backuptarget.h"

BackupTarget::BackupTarget(const std::filesystem::path& path,
                           const std::string& target_name,
                           const std::vector<std::string>& backup_names,
                           const std::vector<int>& active_members) :
  path(path), target_name(target_name), backup_names(backup_names), active_members(active_members)
{}

bool BackupTarget::operator==(const BackupTarget& other) const
{
  for(int i = 0; i < backup_names.size(); i++)
    if(backup_names[i] != other.backup_names[i])
      return false;
  for(int i = 0; i < active_members.size(); i++)
    if(active_members[i] != other.active_members[i])
      return false;
  return path == other.path && target_name == other.target_name &&
         backup_names.size() == other.backup_names.size() &&
         active_members.size() == other.active_members.size();
}
