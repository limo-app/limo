#include "deployerlistmodel.h"
#include "colors.h"
#include "core/deployerentry.hpp"
#include "core/treeitem.h"
#include "modlistmodel.h"
#include "qmodelindexutils.h"
#include <QApplication>
#include <QBrush>

DeployerListModel::DeployerListModel(QObject* parent) : QAbstractItemModel(parent) {
}

QVariant DeployerListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(role == Qt::TextAlignmentRole && section == 2)
    return Qt::AlignLeft;
  if(role == Qt::DisplayRole)
  {
    if(orientation == Qt::Orientation::Vertical)
      return QString::number(section + 1);
    if(section == name_col)
      return QString("Name");
    if(section == id_col)
    {
      if(!deployer_info_.ids_are_source_references)
        return QString("ID");
      else
        return QString("Source Mod");
    }
    if(section == tags_col)
      return QString("Tags");
  }
  return QVariant();
}

int DeployerListModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid() && deployer_info_.root == nullptr)
    return 0;

  const auto parentItem = parent.isValid()
    ? qModelIndexToShared<TreeItem<DeployerEntry>>(parent)
    : deployer_info_.root;

  return parentItem->childCount();
}

int DeployerListModel::columnCount(const QModelIndex& parent) const
{
  return 3;
}

QVariant DeployerListModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  const int row = index.row();
  const int col = index.column();
  auto entry = qModelIndexToShared<TreeItem<DeployerEntry>>(index);
  auto modinfo = qModelIndexToShared<TreeItem<DeployerModInfo>>(index)->getData();
  auto data = entry->getData();
  if (role == Qt::CheckStateRole)
  {
    if (index.column() == name_col && !data->isSeparator)
      return modinfo->enabled ? Qt::Checked : Qt::Unchecked;
    else
      return QVariant();
  }
  if(role == Qt::ForegroundRole)
  {
    if(!text_colors_.contains(modinfo->id))
      return QApplication::palette().text();
    return text_colors_.at(modinfo->id);
  }
  if(role == Qt::DisplayRole)
  {
    if(col == name_col)
    {
      return data->name.c_str();
    }
    if(col == id_col)
    {
      if (!data->isSeparator) {
        const int id = modinfo->id;
        if(!deployer_info_.ids_are_source_references)
          return id;
        if(id == -1)
          return modinfo->sourceName.c_str();
        return std::format("{} [{}]", deployer_info_.source_mod_names_[row], id).c_str();
      }
      return QString("");
    }
    if(col == tags_col)
    {
      auto tags = collectTags(entry);
      return tags.join(", ");
    }
  }
  if(role == mod_status_role)
  {
    if (!data->isSeparator) {
      return modinfo->enabled;
    } else {
      return false;
    }
  }
  if(role == ModListModel::mod_id_role)
  {
    if(!deployer_info_.ids_are_source_references && !data->isSeparator)
      return modinfo->id;
    return row;
  }
  if(role == ModListModel::mod_name_role || role == Qt::EditRole)
    return data->name.c_str();
  if(role == mod_tags_role)
  {
    QStringList tags = collectTags(entry);
    return tags;
  }
  if(role == ids_are_source_references_role)
    return deployer_info_.ids_are_source_references;
  if(role == source_mod_name_role)
  {
    if(deployer_info_.ids_are_source_references && !data->isSeparator)
      return modinfo->sourceName.c_str();
    else
      return data->name.c_str();
  }
  if(role == valid_mod_actions_role)
  {
    QVariant var;
    var.setValue<std::vector<int>>(deployer_info_.valid_mod_actions[row]);
    return var;
  }
  if (role == ModListModel::expansion_role) {
    if (deployer_info_.supports_expandable) {
      if (data->isSeparator) {
      return data->isExpanded;
      } else {
      return false; // Non-separator items are not expandable
      }
    }
    return false;
  }
  return QVariant();
}

void DeployerListModel::setDeployerInfo(const DeployerInfo& info)
{
  emit layoutAboutToBeChanged();
  deployer_info_ = info;
  for(int group = 0; group < info.conflict_groups.size(); group++)
  {
    for(int mod_id : info.conflict_groups[group])
    {
      QBrush color(group % 2 == 0 ? colors::LIGHT_BLUE : colors::ORANGE);
      if(group == info.conflict_groups.size() - 1)
        color = QApplication::palette().text();
      text_colors_[mod_id] = color;
    }
  }
  emit layoutChanged();
}

bool DeployerListModel::hasSeparateDirs() const
{
  return deployer_info_.separate_profile_dirs;
}

bool DeployerListModel::hasIgnoredFiles() const
{
  return deployer_info_.has_ignored_files;
}

bool DeployerListModel::usesUnsafeSorting() const
{
  return deployer_info_.uses_unsafe_sorting;
}

QModelIndex DeployerListModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return {};

  auto parentItem = parent.isValid()
    ? qModelIndexToShared<TreeItem<DeployerEntry>>(parent)
    : deployer_info_.root;

  if (auto childItem = parentItem->child(row).get())
    return createIndex(row, column, childItem);
  return {};
}

QModelIndex DeployerListModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return {};

  auto childItem = qModelIndexToShared<TreeItem<DeployerEntry>>(index);
  auto parentItem = childItem->parent();

  return (parentItem != deployer_info_.root) && (parentItem != nullptr)
  ? createIndex(parentItem->row(), 0, parentItem.get()) : QModelIndex{};
}

bool DeployerListModel::hasChildren(const QModelIndex &parent = QModelIndex()) const
{
  if (deployer_info_.supports_expandable) {
    if (parent.isValid()) {
      auto item = static_cast<TreeItem<DeployerModInfo> *>(parent.internalPointer());
      if (item != nullptr && item->getData()->isSeparator) {
        return true;
      }
      return false;
    } else {
      return true;
    }
  } else {
    if (!parent.isValid()) {
      return true;
    } else {
      return false;
    }
  }
}

bool DeployerListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole && role != Qt::CheckStateRole)
        return false;

    auto item = qModelIndexToShared<TreeItem<DeployerEntry>>(index)->getData();
    if (role == Qt::CheckStateRole) {
      auto state = static_cast<Qt::CheckState>(value.toInt());
      auto modinfo = qModelIndexToShared<TreeItem<DeployerModInfo>>(index)->getData();
      modinfo->enabled = (state == Qt::Checked);
      return true;
    } else
      item->name = value.toString().toStdString();
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole, Qt::CheckStateRole});

    return value.toString() == item->name.c_str();
}

void DeployerListModel::addSeparator()
{
  emit layoutAboutToBeChanged();
  if (deployer_info_.supports_expandable) {
    this->deployer_info_.root->emplace_back(
      std::make_shared<DeployerEntry>(true, "Separator")
    );
  }
  emit layoutChanged();
}

Qt::ItemFlags DeployerListModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  auto flags = QAbstractItemModel::flags(index);
  auto item = qModelIndexToShared<TreeItem<DeployerEntry>>(index);
  if (item->getData()->isSeparator && (index.column() == name_col))
    flags |= Qt::ItemIsEditable;
  else if (index.column() == name_col)
    flags |= Qt::ItemIsUserCheckable;
  return flags;
}

QStringList DeployerListModel::collectTags(std::shared_ptr<TreeItem<DeployerEntry>> item) const {
  QStringList tags;
  auto info = item->getData();
  if (!info->isSeparator) {
    auto modinfo = reinterpret_pointer_cast<TreeItem<DeployerModInfo>>(item)->getData();
    for(const auto& tag : modinfo->auto_tags)
      tags.push_back(tag.c_str());
    for(const auto& tag : modinfo->manual_tags)
      tags.push_back(tag.c_str());
  }
  else {
    for (int i = 0; i < item->childCount(); ++i) {
      tags += collectTags(item->child(i));
    }
  }
  tags.sort(Qt::CaseInsensitive);
  tags.removeDuplicates();
  return tags;
};
