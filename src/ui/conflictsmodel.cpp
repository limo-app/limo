#include "conflictsmodel.h"
#include "colors.h"


ConflictsModel::ConflictsModel(QObject* parent) : QAbstractTableModel(parent) {}

QVariant ConflictsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(role == Qt::TextAlignmentRole && section == id_col)
    return Qt::AlignLeft;
  if(role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    if(section == file_col)
      return QString("File");
    if(section == name_col)
      return QString("Winner name");
    if(section == id_col)
      return QString("Winner ID");
  }
  return QVariant();
}

int ConflictsModel::rowCount(const QModelIndex& parent) const
{
  if(parent.isValid())
    return 0;
  return conflicts_.size();
}

int ConflictsModel::columnCount(const QModelIndex& parent) const
{
  if(parent.isValid())
    return 0;
  return 3;
}

QVariant ConflictsModel::data(const QModelIndex& index, int role) const
{
  if(!index.isValid())
    return QVariant();

  const int row = index.row();
  const int col = index.column();

  if(role == Qt::DisplayRole)
  {
    if(col == file_col)
      return conflicts_[row].file.c_str();
    if(col == name_col)
      return conflicts_[row].mod_name.c_str();
    if(col == id_col)
      return QString::number(conflicts_[row].mod_id);
  }

  if(role == Qt::ForegroundRole)
    return conflicts_[row].mod_id == base_id_ ? colors::GREEN : colors::RED;

  return QVariant();
}

void ConflictsModel::setConflicts(const std::vector<ConflictInfo>& newConflicts, int base_id)
{
  emit layoutAboutToBeChanged();
  conflicts_ = newConflicts;
  base_id_ = base_id;
  emit layoutChanged();
}
