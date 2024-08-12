#include "backuplistmodel.h"
#include <QApplication>
#include <QPainter>
#include <QTableView>


BackupListModel::BackupListModel(QObject* parent) : QAbstractTableModel(parent) {}

QVariant BackupListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(role == Qt::TextAlignmentRole && section == path_col)
    return Qt::AlignLeft;
  if(role == Qt::DisplayRole)
  {
    if(section == action_col)
      return QString("Action");
    if(section == target_col)
      return QString("Target");
    if(section == backup_col)
      return QString("Backup");
    if(section == path_col)
      return QString("Path");
  }
  return QVariant();
}

int BackupListModel::rowCount(const QModelIndex& parent) const
{
  return targets_.size() + 1;
}

int BackupListModel::columnCount(const QModelIndex& parent) const
{
  return 4;
}

QVariant BackupListModel::data(const QModelIndex& index, int role) const
{
  // 0:Action | 1:Target | 2:Backup | 3:Path
  if(!index.isValid())
    return QVariant();
  const int row = index.row();
  const int col = index.column();
  if(row == targets_.size())
  {
    if(role == ModListModel::icon_role && col == action_col)
      return QIcon::fromTheme("list-add");
    return QVariant();
  }

  if(role == ModListModel::icon_role && col == action_col)
    return QIcon::fromTheme("user-trash");
  if(role == Qt::DisplayRole)
  {
    if(col == target_col)
      return QString::fromStdString(targets_[row].target_name);
    if(col == backup_col)
      return QString::fromStdString(targets_[row].backup_names[targets_[row].cur_active_member]);
    if(col == path_col)
      return QString::fromStdString(targets_[row].path);
  }
  if(role == backup_list_role)
  {
    QStringList backups_list;
    for(const auto& backup : targets_[row].backup_names)
      backups_list.append(backup.c_str());
    return backups_list;
  }
  if(role == active_index_role)
    return targets_[row].cur_active_member;
  if(role == num_backups_role)
  {
    QVariant var;
    var.setValue<size_t>(targets_[row].backup_names.size());
    return var;
  }
  if(role == num_targets_role)
  {
    QVariant var;
    var.setValue<size_t>(targets_.size());
    return var;
  }
  if(role == target_name_role && row < targets_.size())
    return QString::fromStdString(targets_[row].target_name);
  if(role == backup_name_role && row < targets_.size())
    return QString::fromStdString(targets_[row].backup_names[targets_[row].cur_active_member]);
  if(role == target_path_role)
    return targets_[row].path.c_str();
  return QVariant();
}

Qt::ItemFlags BackupListModel::flags(const QModelIndex& index) const
{
  if(!index.isValid())
    return Qt::NoItemFlags;

  const int col = index.column();
  const int row = index.row();
  if(is_editable_ && (col == backup_col || col == target_col) && row < targets_.size())
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

  return QAbstractItemModel::flags(index) & ~Qt::ItemIsEditable;
}

void BackupListModel::setBackupTargets(const std::vector<BackupTarget>& targets)
{
  emit layoutAboutToBeChanged();
  targets_ = targets;
  emit layoutChanged();
}

void BackupListModel::setIsEditable(bool is_editable)
{
  is_editable_ = is_editable;
}

bool BackupListModel::isEditable() const
{
  return is_editable_;
}
