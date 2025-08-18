#include "deployerlistproxymodel.h"
#include "colors.h"
#include "deployerlistmodel.h"
#include "modlistmodel.h"
#include <QApplication>
#include <ranges>

namespace str = std::ranges;


DeployerListProxyModel::DeployerListProxyModel(QLabel* row_count_label, QObject* parent) :
  QSortFilterProxyModel(parent), row_count_label_(row_count_label),
  id_regex_(R"(id:\s*(\d+))", QRegularExpression::CaseInsensitiveOption)
{}

QVariant DeployerListProxyModel::data(const QModelIndex& index, int role) const
{
  if(role == Qt::ForegroundRole)
  {
    const int row = index.row();
    const int col = index.column();
    if(row >= row_text_colors_.size())
      return QApplication::palette().text();
    return row_text_colors_[row];
  }
  else
    return sourceModel()->data(mapToSource(index), role);
}

bool DeployerListProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  return sourceModel()->setData(mapToSource(index), value, role);
}

void DeployerListProxyModel::setFilterMode(FilterMode mode, bool status, bool invalidate_filter)
{
  if(status)
    addFilter(mode, false);
  else
    removeFilter(mode, false);
  if(invalidate_filter)
    updateFilter();
}

void DeployerListProxyModel::addFilter(FilterMode mode, bool invalidate_filter)
{
  if(!(filter_mode_ & mode))
    filter_mode_ += mode;
  if((filter_mode_ & filter_active) && (filter_mode_ & filter_inactive))
    filter_mode_ -= filter_active + filter_inactive - mode;
  if(invalidate_filter)
    updateFilter();
}

void DeployerListProxyModel::removeFilter(FilterMode mode, bool invalidate_filter)
{
  if(filter_mode_ & mode)
    filter_mode_ -= mode;
  if(mode == filter_tags)
    tag_filters_.clear();
  if(invalidate_filter)
    updateFilter();
}

bool DeployerListProxyModel::filterAcceptsRow(int source_row,
                                              const QModelIndex& source_parent) const
{
  bool show = true;
  const auto index = sourceModel()->index(source_row, 0, source_parent);
  if(filter_string_targets_id_)
  {
    show *=
      QString::number(index.data(ModListModel::mod_id_role).toInt()).contains(filter_string_id_);
  }
  else if(filter_string_is_int_)
  {
    show *=
      QString::number(index.data(ModListModel::mod_id_role).toInt()).contains(filter_string_);
    show |= index.data(ModListModel::mod_name_role)
              .toString()
              .contains(filter_string_, Qt::CaseInsensitive);
  }
  else
  {
    show *= index.data(ModListModel::mod_name_role)
              .toString()
              .contains(filter_string_, Qt::CaseInsensitive);
    if(index.data(DeployerListModel::ids_are_source_references_role).toBool())
    {
      show |= index.data(DeployerListModel::source_mod_name_role)
                .toString()
                .contains(filter_string_, Qt::CaseInsensitive);
    }
  }
  if(filter_mode_ & filter_active)
    show *= index.data(DeployerListModel::mod_status_role).toBool();
  else if(filter_mode_ & filter_inactive)
    show *= !index.data(DeployerListModel::mod_status_role).toBool();
  if(filter_mode_ & filter_conflicts)
    show *= conflicts_.contains(index.data(ModListModel::mod_id_role).toInt());
  if(filter_mode_ & filter_tags)
  {
    const auto tags = index.data(DeployerListModel::mod_tags_role).toStringList();
    for(const auto& [tag, enabled] : tag_filters_)
    {
      const bool contains_tag = tags.contains(tag);
      show *= contains_tag && enabled || !contains_tag && !enabled;
    }
  }
  return show;
}

void DeployerListProxyModel::clearFilter(bool invalidate_filter)
{
  filter_mode_ = 0;
  conflicts_.clear();
  tag_filters_.clear();
  if(invalidate_filter)
    updateFilter();
}

int DeployerListProxyModel::getFilterMode()
{
  return filter_mode_;
}

void DeployerListProxyModel::setConflicts(const std::unordered_set<int>& conflicts)
{
  conflicts_ = conflicts;
}

void DeployerListProxyModel::addTagFilter(const QString& tag, bool include, bool invalidate_filter)
{
  auto filter = str::find_if(tag_filters_, [tag](const auto& pair) { return pair.first == tag; });
  if(filter != tag_filters_.end())
    filter->second = include;
  else
    tag_filters_.emplace_back(tag, include);
  if(invalidate_filter)
    updateFilter();
}

void DeployerListProxyModel::removeTagFilter(const QString& tag, bool invalidate_filter)
{
  auto filter = str::find_if(tag_filters_, [tag](const auto& pair) { return pair.first == tag; });
  if(filter == tag_filters_.end())
    return;
  tag_filters_.erase(filter);
  if(invalidate_filter)
    updateFilter();
}

void DeployerListProxyModel::setConflictGroups(const std::vector<std::vector<int>>& groups)
{
  conflict_groups_.clear();
  no_conflict_group_ = 0;
  for(int group = 0; group < groups.size(); group++)
  {
    no_conflict_group_ = std::max(group, no_conflict_group_);
    for(int mod_id : groups[group])
      conflict_groups_[mod_id] = group;
  }
  updateFilter(false);
}

void DeployerListProxyModel::updateRowCountLabel()
{
  row_count_label_->setText("Mods displayed: " + QString::number(rowCount()));
}

void DeployerListProxyModel::updateFilter(bool invalidate)
{
  if(invalidate)
    invalidateFilter();

  row_text_colors_.clear();
  int prev_group = -1;
  int misses = 0;
  const auto colors = std::vector<QBrush>{ QBrush(colors::LIGHT_BLUE), QBrush(colors::ORANGE) };
  const auto default_color = QApplication::palette().text();
  for(int row = 0; row < rowCount(); row++)
  {
    const int mod_id =
      sourceModel()->data(
        mapToSource(index(row, 0)),
        ModListModel::mod_id_role
      ).toInt();
    const int group = conflict_groups_[mod_id];
    if(group == no_conflict_group_)
    {
      row_text_colors_.push_back(default_color);
      continue;
    }
    if(group != prev_group)
    {
      misses++;
      prev_group = group;
    }
    row_text_colors_.push_back(colors.at(misses % 2));
  }
  updateRowCountLabel();
}

std::vector<std::pair<QString, bool>> DeployerListProxyModel::getTagFilters() const
{
  return tag_filters_;
}

void DeployerListProxyModel::setFilterString(const QString& filter_string)
{
  filter_string_ = filter_string;
  filter_string.toInt(&filter_string_is_int_);
  if(!filter_string_is_int_)
  {
    const QRegularExpressionMatch match = id_regex_.match(filter_string);
    if(match.hasMatch())
    {
      filter_string_targets_id_ = true;
      filter_string_id_ = match.captured(1);
    }
    else
      filter_string_targets_id_ = false;
  }
  invalidateFilter();
}
