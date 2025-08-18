#include "deployerlistview.h"
#include "deployerlistmodel.h"
#include "deployerlistproxymodel.h"
#include "modlistmodel.h"
#include "hotspot.h"
#include "qmodelindexutils.h"
#include <QGuiApplication>

DeployerListView::DeployerListView(QWidget* parent) : ModListView(parent)
{
  setTreePosition(0);
  QFile styleFile(":/styles/tablecellstyle.qss");
  styleFile.open(QFile::ReadOnly | QFile::Text);
  setStyleSheet(styleFile.readAll());
  connect(this, &DeployerListView::expanded,
          this, &DeployerListView::onExpanded);
  connect(this, &DeployerListView::collapsed,
          this, &DeployerListView::onCollapsed);
}

void DeployerListView::setModel(QAbstractItemModel* model)
{
  // Expansion state is stored in the source model and serialized in the config, so we need to restore it on each layout change
  connect(model, &QAbstractItemModel::layoutChanged,
          this, [this]() {
            QTimer *timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->start();
            this->expandSeparators(this->rootIndex());
          });
  QTreeView::setModel(model);
}

void DeployerListView::mousePressEvent(QMouseEvent* event)
{
  QTreeView::mousePressEvent(event);
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const auto prev_index = selectionModel()->currentIndex();
  selectionModel()->clearSelection();
  auto leftIndex = model()->index(event_row, 0, index.parent());
  auto rightIndex = model()->index(event_row, model()->columnCount() - 1, index.parent());
  const auto selection = QItemSelection(leftIndex, rightIndex);
  selectionModel()->select(selection, QItemSelectionModel::Select);
  selectionModel()->setCurrentIndex(model()->index(event_row, 0, index.parent()),
                                    QItemSelectionModel::NoUpdate);
  updateMouseDownRow(index);
  updateRow(prev_index);
}

void DeployerListView::mouseMoveEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int row = index.row();
  if(QGuiApplication::mouseButtons().testFlag(Qt::LeftButton) && enable_drag_reorder_ &&
     mouse_down_.isValid())
  {
    setCursor(Qt::ClosedHandCursor);
    is_in_drag_drop_ = true;
  }
  int mouse_row_region_curr = ROW_REGION.NONE;
  auto rect = visualRect(indexAt(event->pos()));

  auto rowHeight = rect.bottom() - rect.top();
  if(inHotspot(rect, event->pos()))
    mouse_row_region_curr = ROW_REGION.HOTSPOT;
  else if(event->pos().y() < rect.top() + rowHeight / 2)
    mouse_row_region_curr = ROW_REGION.UPPER;
  else if(event->pos().y() > rect.top() + rowHeight / 2)
    mouse_row_region_curr = ROW_REGION.LOWER;

  if(mouse_row_region_curr != mouse_row_region)
  {
    mouse_row_region = mouse_row_region_curr;
    if(is_in_drag_drop_)
    {
      for (const auto i : {
        indexAbove(indexAbove(index)),
        indexAbove(index),
        index,
        indexBelow(index),
        indexBelow(indexBelow(index)) }) {
        updateRow(i);
      }
    }
  }
  updateMouseHoverRow(index);
}

void DeployerListView::mouseReleaseEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int event_col = index.column();
  auto rect = visualRect(index);

  bool reorder = true;
  if (inHotspot(rect, event->pos()))
  {
    reorder = false;
  }

  const bool was_in_drag_drop_ = is_in_drag_drop_;
  is_in_drag_drop_ = false;
  setCursor(Qt::ArrowCursor);
  if(enable_drag_reorder_ && event_row > -1 && was_in_drag_drop_)
  {
    QModelIndex target = index;
    if(target != mouse_down_ && target.isValid() && mouse_down_.isValid())
    {
      auto target_src = static_cast<DeployerListProxyModel*>(model())->mapToSource(target);
      auto mouse_down_src = static_cast<DeployerListProxyModel*>(model())->mapToSource(mouse_down_);

      auto target_entry = qModelIndexToShared<TreeItem<DeployerEntry>>(target_src);
      auto mouse_down_entry = qModelIndexToShared<TreeItem<DeployerEntry>>(mouse_down_src);

      if (reorder)
      {
        int position = -1;
        auto target_parent = target_entry->parent();
        if (target_entry->getData()->isSeparator && mouse_row_region == ROW_REGION.LOWER && isExpanded(index))
        {
            position = 0;
            target_parent = target_entry;
        }
        else {
          if (mouse_row_region == ROW_REGION.UPPER)
            position = target.row();
          else if (mouse_row_region == ROW_REGION.LOWER)
            position = target.row()+1;
        }
        rowsAboutToBeRemoved(mouse_down_.parent(), mouse_down_.row(), mouse_down_.row());
        // make null and then remove after
        auto mouse_down_owned = mouse_down_entry->parent()->markNull(mouse_down_entry);
        target_parent->insert(position, mouse_down_owned);
        mouse_down_entry->parent()->remove(std::shared_ptr<TreeItem<DeployerEntry>>());
        mouse_down_entry->setParent(target_parent);
        emit modMoved();
        selectionModel()->setCurrentIndex(model()->index(position, 0, target.parent()),
                                          QItemSelectionModel::SelectCurrent);
      }
      else if (mouse_row_region == ROW_REGION.HOTSPOT && target_entry->getData()->isSeparator)
      {
        rowsAboutToBeRemoved(mouse_down_.parent(), mouse_down_.row(), mouse_down_.row());
        // target is separator/category
        auto mouse_down_owned = mouse_down_entry->parent()->markNull(mouse_down_entry);
        target_entry->emplace_back(mouse_down_owned);
        mouse_down_entry->parent()->remove(std::shared_ptr<TreeItem<DeployerEntry>>());
        mouse_down_entry->setParent(target_entry);
        emit modMoved();
        selectionModel()->setCurrentIndex(model()->index(target_entry->childCount()-1, 0, target),
                                          QItemSelectionModel::SelectCurrent);
      }
      updateMouseDownRow(target);
    }
    else
    {
      updateRow(index);
      updateRow(indexAbove(index));
      updateRow(indexBelow(index));
    }
  }
  if(!sameRow(index, mouse_down_))
    updateMouseHoverRow(QModelIndex());
  if (index.column() == 0)
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole, Qt::CheckStateRole});
  QTreeView::mouseReleaseEvent(event);
}

bool DeployerListView::enableDragReorder() const
{
  return enable_drag_reorder_;
}

void DeployerListView::setEnableDragReorder(bool enabled)
{
  enable_drag_reorder_ = enabled;
}

void DeployerListView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) 
{
  updateRow(topLeft);
  emit modMoved();
  QTreeView::dataChanged(topLeft, bottomRight, roles);
}

void DeployerListView::mouseDoubleClickEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  auto index_proxy = static_cast<DeployerListProxyModel*>(model())->mapToSource(index);
  auto entry = qModelIndexToShared<TreeItem<DeployerEntry>>(index_proxy);
  const int event_col = index.column();
  if(event->button() == Qt::LeftButton
    && event_col == DeployerListModel::name_col
    && entry->getData()->isSeparator)
    edit(index);
}

void DeployerListView::onExpanded(const QModelIndex &index) {
  auto index_proxy = static_cast<DeployerListProxyModel*>(model())->mapToSource(index);
  auto entry = qModelIndexToShared<TreeItem<DeployerEntry>>(index_proxy);
  auto data = entry->getData();
  data->isExpanded = true;
}

void DeployerListView::onCollapsed(const QModelIndex &index) {
  auto index_proxy = static_cast<DeployerListProxyModel*>(model())->mapToSource(index);
  auto entry = qModelIndexToShared<TreeItem<DeployerEntry>>(index_proxy);
  auto data = entry->getData();
  data->isExpanded = false;
}

void DeployerListView::expandSeparators(const QModelIndex &index)
{
  bool shouldExpand = this->model()->data(
    index, ModListModel::expansion_role).toBool();
  if (shouldExpand) {
    expand(index);
  }

  const int rowCount = model()->rowCount(index);
  for (int row = 0; row < rowCount; ++row) {
    const QModelIndex child = model()->index(row, 0, index);
    if (!child.isValid())
      break;
    if (this->isExpanded(child))
      continue;
    expandSeparators(child);
  }
}
