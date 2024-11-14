#include "conflictsmodel.h"
#include "colors.h"
#include <ranges>


ConflictsModel::ConflictsModel(QObject* parent) : QAbstractTableModel(parent) {}

QVariant ConflictsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(role == Qt::TextAlignmentRole && section == order_col)
    return Qt::AlignLeft;
  if(role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    if(section == file_col)
      return QString("File");
    if(section == winner_col)
      return QString("Winner");
    if(section == order_col)
      return QString("Overwrite order");
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
    if(col == winner_col)
    {
      return std::format(
               "{} [{}]", conflicts_[row].mod_names.back(), conflicts_[row].mod_ids.back())
        .c_str();
    }
    if(col == order_col)
    {
      QStringList result;
      for(const auto& [name, id] : std::views::zip(conflicts_[row].mod_names, conflicts_[row].mod_ids))
        result.append(std::format("{} [{}]", name, id).c_str());
      return result.join(" ==> ");
    }
  }

  if(role == Qt::ForegroundRole)
    return conflicts_[row].mod_ids.back() == base_id_ ? colors::GREEN : colors::RED;

  return QVariant();
}

void ConflictsModel::setConflicts(const std::vector<ConflictInfo>& newConflicts, int base_id)
{
  emit layoutAboutToBeChanged();
  conflicts_ = newConflicts;
  base_id_ = base_id;
  emit layoutChanged();
}
