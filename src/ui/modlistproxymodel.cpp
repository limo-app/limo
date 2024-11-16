#include "modlistproxymodel.h"
#include "modlistmodel.h"
#include <ranges>

namespace str = std::ranges;


ModListProxyModel::ModListProxyModel(QLabel* row_count_label, QObject* parent) :
  QSortFilterProxyModel(parent), row_count_label_(row_count_label),
  id_regex_(R"(id:\s*(\d+))", QRegularExpression::CaseInsensitiveOption)
{}

QVariant ModListProxyModel::data(const QModelIndex& index, int role) const
{
  return sourceModel()->data(mapToSource(index), role);
}

bool ModListProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  return sourceModel()->setData(mapToSource(index), value, role);
}

void ModListProxyModel::setFilterMode(FilterMode mode, bool status, bool invalidate_filter)
{
  if(status)
    addFilter(mode, false);
  else
    removeFilter(mode, false);
  if(invalidate_filter)
    invalidateFilter();
  updateRowCountLabel();
}

void ModListProxyModel::addFilter(FilterMode mode, bool invalidate_filter)
{
  if(!(filter_mode_ & mode))
    filter_mode_ += mode;
  if((filter_mode_ & filter_active) && (filter_mode_ & filter_inactive))
    filter_mode_ -= filter_active + filter_inactive - mode;
  else if((filter_mode_ & filter_groups) && (filter_mode_ & filter_no_groups))
    filter_mode_ -= filter_groups + filter_no_groups - mode;
  else if((filter_mode_ & filter_updates) && (filter_mode_ & filter_no_updates))
    filter_mode_ -= filter_updates + filter_no_updates - mode;
  if(invalidate_filter)
    invalidateFilter();
  updateRowCountLabel();
}

void ModListProxyModel::removeFilter(FilterMode mode, bool invalidate_filter)
{
  if(filter_mode_ & mode)
    filter_mode_ -= mode;
  if(mode == filter_tags)
    tag_filters_.clear();
  if(invalidate_filter)
    invalidateFilter();
  updateRowCountLabel();
}

bool ModListProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
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
  }
  if(filter_mode_ & filter_groups)
    show *= index.data(ModListModel::mod_group_role).toInt() >= 0;
  else if(filter_mode_ & filter_no_groups)
    show *= index.data(ModListModel::mod_group_role).toInt() == -1;
  if(filter_mode_ & filter_active)
  {
    const auto statuses = index.data(ModListModel::statuses_role).value<std::vector<bool>>();
    if(statuses.empty())
      show = false;
    else
      show *= str::max(statuses);
  }
  else if(filter_mode_ & filter_inactive)
  {
    const auto statuses = index.data(ModListModel::statuses_role).value<std::vector<bool>>();
    if(!statuses.empty())
      show *= !str::max(statuses);
  }
  if(filter_mode_ & filter_tags)
  {
    const auto manual_tags = index.data(ModListModel::manual_tags_role).toStringList();
    const auto auto_tags = index.data(ModListModel::auto_tags_role).toStringList();
    for(const auto& [tag, enabled] : tag_filters_)
    {
      const bool contains_tag = manual_tags.contains(tag) || auto_tags.contains(tag);
      show *= contains_tag && enabled || !contains_tag && !enabled;
    }
  }
  if(filter_mode_ & filter_updates)
    show *= index.data(ModListModel::has_update_role).toBool();
  if(filter_mode_ & filter_no_updates)
    show *= !index.data(ModListModel::has_update_role).toBool();
  return show;
}

void ModListProxyModel::clearFilter(bool invalidate_filter)
{
  filter_mode_ = 0;
  tag_filters_.clear();
  if(invalidate_filter)
    invalidateFilter();
  updateRowCountLabel();
}

int ModListProxyModel::getFilterMode()
{
  return filter_mode_;
}

void ModListProxyModel::addTagFilter(const QString& tag, bool include, bool invalidate_filter)
{
  auto filter = str::find_if(tag_filters_, [tag](const auto& pair) { return pair.first == tag; });
  if(filter != tag_filters_.end())
    filter->second = include;
  else
    tag_filters_.emplace_back(tag, include);
  if(invalidate_filter)
    invalidateFilter();
  updateRowCountLabel();
}

void ModListProxyModel::removeTagFilter(const QString& tag, bool invalidate_filter)
{
  auto filter = str::find_if(tag_filters_, [tag](const auto& pair) { return pair.first == tag; });
  if(filter == tag_filters_.end())
    return;
  tag_filters_.erase(filter);
  if(invalidate_filter)
    invalidateFilter();
  updateRowCountLabel();
}

void ModListProxyModel::updateRowCountLabel()
{
  row_count_label_->setText("Mods displayed: " + QString::number(rowCount()));
}

bool ModListProxyModel::isEditable() const
{
  return static_cast<ModListModel*>(sourceModel())->isEditable();
}

bool ModListProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  const int col = left.column();
  if(col == ModListModel::size_col)
  {
    const auto left_size = left.data(ModListModel::mod_size_role).value<unsigned long>();
    const auto right_size = right.data(ModListModel::mod_size_role).value<unsigned long>();
    return left_size < right_size;
  }
  return QSortFilterProxyModel::lessThan(left, right);
}

std::vector<std::pair<QString, bool>> ModListProxyModel::getTagFilters() const
{
  return tag_filters_;
}

void ModListProxyModel::setFilterString(const QString& filter_string)
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
