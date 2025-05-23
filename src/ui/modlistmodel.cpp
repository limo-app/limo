#include "modlistmodel.h"
#include "colors.h"
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QPainter>
#include <QTableView>


ModListModel::ModListModel(ModListProxyModel* proxy, QObject* parent) :
  QStandardItemModel(parent), proxy_model_(proxy)
{}

QVariant ModListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(role == Qt::TextAlignmentRole && section == deployers_col)
    return Qt::AlignLeft;
  if(role == Qt::DisplayRole)
  {
    if(orientation == Qt::Orientation::Vertical)
      return QString::number(section + 1);
    if(section == action_col)
      return QString("Action");
    if(section == name_col)
      return QString("Name");
    if(section == version_col)
      return QString("Version");
    if(section == id_col)
      return QString("ID");
    if(section == time_col)
      return QString("Installation Time");
    if(section == size_col)
      return QString("Size");
    if(section == deployers_col)
      return QString("Deployers");
    if(section == tags_col)
      return QString("Tags");
  }
  return QVariant();
}

int ModListModel::rowCount(const QModelIndex& parent) const
{
  return active_mods_.size();
}

int ModListModel::columnCount(const QModelIndex& parent) const
{
  return 8;
}

QVariant ModListModel::data(const QModelIndex& index, int role) const
{
  // 0:Action | 1:Name | 2:Version | 3:ID | 4:Installation Time | 5:Deployers
  if(!index.isValid())
    return QVariant();
  const int row = index.row();
  const int col = index.column();

  if(role == icon_role && col == action_col)
    return QIcon::fromTheme("user-trash");

  if(role == Qt::DisplayRole || role == sort_role)
  {
    if(col == name_col)
      return QString::fromStdString(active_mods_[row].mod.name);
    if(col == version_col)
      return QString::fromStdString(active_mods_[row].mod.version);
    if(col == id_col && role == Qt::DisplayRole)
      return QString::number(active_mods_[row].mod.id);
    if(col == id_col && role == sort_role)
      return active_mods_[row].mod.id;
    if(col == time_col)
    {
      std::stringstream ss;
      ss << std::put_time(std::localtime(&active_mods_[row].mod.install_time), "%F %T");
      return QString::fromStdString(ss.str());
    }
    if(col == size_col)
      return mod_size_strings_.at(active_mods_[row].mod.id);
    if(col == deployers_col)
    {
      QStringList deployers;
      for(const auto& depl : active_mods_[row].deployers)
        deployers.append(depl.c_str());
      return deployers.join(", ");
    }
    if(col == tags_col)
    {
      QStringList tags;
      for(const auto& tag : manual_tag_map_.at(active_mods_[row].mod.id))
        tags.append(tag.c_str());
      for(const auto& tag : auto_tag_map_.at(active_mods_[row].mod.id))
        tags.append(tag.c_str());
      tags.sort(Qt::CaseInsensitive);
      if(modHasUpdate(row))
        tags.insert(0, "[Has Update]");
      return tags.join(", ");
    }
  }
  if(role == Qt::ForegroundRole)
  {
    if(modHasUpdate(row))
      return QBrush(colors::GREEN);
  }
  if(role == version_list_role)
  {
    if(col == version_col)
    {
      const int group = active_mods_[row].group;
      if(group >= 0)
        return group_versions_.at(group);
      else
        return QStringList(active_mods_[row].mod.version.c_str());
    }
  }
  if(role == mod_id_role)
    return active_mods_[row].mod.id;
  if(role == active_index_role)
    return active_group_members_.at(active_mods_[row].group);
  if(role == mod_name_role)
    return active_mods_[row].mod.name.c_str();
  if(role == group_members_role)
  {
    QVariant var;
    var.setValue<std::vector<int>>(groups_.at(active_mods_[row].group));
    return var;
  }
  if(role == mod_group_role)
    return active_mods_[row].group;
  if(role == deployer_ids_role)
  {
    QVariant var;
    var.setValue<std::vector<int>>(active_mods_[row].deployer_ids);
    return var;
  }
  if(role == statuses_role)
  {
    QVariant var;
    var.setValue<std::vector<bool>>(active_mods_[row].deployer_statuses);
    return var;
  }
  if(role == manual_tags_role)
  {
    QStringList tags;
    for(const auto& tag : manual_tag_map_.at(active_mods_[row].mod.id))
      tags.append(tag.c_str());
    return tags;
  }
  if(role == auto_tags_role)
  {
    QStringList tags;
    for(const auto& tag : auto_tag_map_.at(active_mods_[row].mod.id))
      tags.append(tag.c_str());
    return tags;
  }
  if(role == local_source_role)
    return active_mods_.at(row).mod.local_source.c_str();
  if(role == remote_source_role)
    return active_mods_.at(row).mod.remote_source.c_str();
  if(role == has_update_role)
    return modHasUpdate(row);
  if(role == mod_size_role)
  {
    QVariant var;
    var.setValue<unsigned long>(active_mods_.at(row).mod.size_on_disk);
    return var;
  }
  if(role == mod_version_role)
    return active_mods_[row].mod.version.c_str();
  return QVariant();
}

QVariant ModListModel::data(int role, int row, int col) const
{
  return data(index(row, col), role);
}

Qt::ItemFlags ModListModel::flags(const QModelIndex& index) const
{
  if(!index.isValid())
    return Qt::NoItemFlags;
  if(is_editable_ && (index.column() == 2 || index.column() == 1))
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  return QAbstractItemModel::flags(index) & ~Qt::ItemIsEditable;
}

void ModListModel::setModInfo(const std::vector<ModInfo>& mods)
{
  emit layoutAboutToBeChanged();
  mods_ = mods;
  active_mods_.clear();
  active_mods_.reserve(mods.size());
  groups_.clear();
  group_map_.clear();
  group_versions_.clear();
  active_group_members_.clear();
  deployer_statuses_.clear();
  manual_tag_map_.clear();
  auto_tag_map_.clear();
  mod_size_strings_.clear();
  for(const auto& info : mods)
  {
    deployer_statuses_[info.mod.id] = info.deployer_statuses;
    if(info.group >= 0)
    {
      group_map_[info.mod.id] = info.group;
      if(!groups_.contains(info.group))
      {
        groups_[info.group] = { info.mod.id };
        group_versions_[info.group] = QStringList(info.mod.version.c_str());
      }
      else
      {
        groups_[info.group].push_back(info.mod.id);
        group_versions_[info.group].append(info.mod.version.c_str());
      }
      if(info.is_active_group_member)
        active_group_members_[info.group] = groups_[info.group].size() - 1;
    }
    if(info.is_active_group_member || info.group < 0)
      active_mods_.push_back(info);

    manual_tag_map_[info.mod.id] = info.manual_tags;
    auto_tag_map_[info.mod.id] = info.auto_tags;

    unsigned long size = info.mod.size_on_disk;
    unsigned long last_size = 0;
    int exp = 0;
    const std::vector<QString> units{ "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB" };
    QString size_string = "";
    while(size > 1024 && exp < units.size())
    {
      last_size = size;
      size /= 1024;
      exp++;
    }
    last_size /= 1.024;
    size_string = QString::number(size);
    const int first_digit = (last_size / 100) % 10;
    const int second_digit = (last_size / 10) % 10;
    if(first_digit != 0 || second_digit != 0)
      size_string += "." + QString::number(first_digit);
    if(second_digit != 0)
      size_string += QString::number(second_digit);
    size_string += " " + units[exp];
    mod_size_strings_[info.mod.id] = size_string;
  }
  emit layoutChanged();
}

const std::map<int, int>& ModListModel::getGroupMap() const
{
  return group_map_;
}

const std::vector<ModInfo>& ModListModel::getModInfo() const
{
  return mods_;
}

bool ModListModel::isEditable() const
{
  return is_editable_;
}

void ModListModel::setIsEditable(bool is_editable)
{
  is_editable_ = is_editable;
}

bool ModListModel::modHasUpdate(int row) const
{
  return active_mods_.at(row).mod.remote_update_time > active_mods_.at(row).mod.install_time &&
         active_mods_.at(row).mod.remote_update_time >
           active_mods_.at(row).mod.suppress_update_time;
}
