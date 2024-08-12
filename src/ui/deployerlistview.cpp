#include "deployerlistview.h"
#include "deployerlistmodel.h"
#include "deployerlistproxymodel.h"
#include "modlistmodel.h"
#include <QGuiApplication>


DeployerListView::DeployerListView(QWidget* parent) : ModListView(parent)
{
  setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void DeployerListView::mousePressEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int prev_row = selectionModel()->currentIndex().row();
  selectionModel()->clearSelection();
  const auto selection = QItemSelection(model()->index(event_row, 1),
                                        model()->index(event_row, model()->columnCount() - 1));
  selectionModel()->select(selection, QItemSelectionModel::Select);
  selectionModel()->setCurrentIndex(model()->index(event_row, 1),
                                    QItemSelectionModel::NoUpdate);
  updateMouseDownRow(event_row);
  updateRow(prev_row);
}

void DeployerListView::mouseReleaseEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int event_col = index.column();

  const bool was_in_drag_drop_ = is_in_drag_drop_;
  is_in_drag_drop_ = false;
  setCursor(Qt::ArrowCursor);
  if(enable_drag_reorder_ && event_row > -1 && was_in_drag_drop_)
  {
    int target_row = mouse_in_upper_half_of_row_ ? event_row : event_row + 1;
    if(mouse_down_row_ < target_row)
      target_row--;
    target_row = std::min(target_row, model()->rowCount());
    if(target_row != mouse_down_row_ && rowIndexIsValid(target_row) &&
       rowIndexIsValid(mouse_down_row_))
    {
      const auto from_row = static_cast<DeployerListProxyModel*>(model())
                              ->mapToSource(model()->index(mouse_down_row_, 0))
                              .row();
      const auto to_row = static_cast<DeployerListProxyModel*>(model())
                            ->mapToSource(model()->index(target_row, 0))
                            .row();
      emit modMoved(from_row, to_row);
      selectionModel()->setCurrentIndex(model()->index(target_row, 1),
                                        QItemSelectionModel::SelectCurrent);
      updateMouseDownRow(target_row);
    }
    else
    {
      updateRow(event_row);
      updateRow(event_row - 1);
      updateRow(event_row + 1);
    }
  }
  if(event_row != mouse_down_row_)
    updateMouseHoverRow(-1);
  else if(event_col == DeployerListModel::status_col && event_row > -1 &&
          event_row < model()->rowCount() && enable_buttons_ && !was_in_drag_drop_)
  {
    emit modStatusChanged(model()->data(index, ModListModel::mod_id_role).toInt(),
                          !model()->data(index, DeployerListModel::mod_status_role).toBool());
  }
  //  QTableView::mouseReleaseEvent(event);
}

void DeployerListView::mouseMoveEvent(QMouseEvent* event)
{
  const int row = indexAt(event->pos()).row();
  if(QGuiApplication::mouseButtons().testFlag(Qt::LeftButton) && enable_drag_reorder_ &&
     mouse_down_row_ != -1)
  {
    setCursor(Qt::ClosedHandCursor);
    is_in_drag_drop_ = true;
  }
  bool mouse_in_upper_half = true;
  if(event->pos().y() > rowViewportPosition(row) + rowHeight(row) / 2)
    mouse_in_upper_half = false;
  if(mouse_in_upper_half != mouse_in_upper_half_of_row_)
  {
    mouse_in_upper_half_of_row_ = mouse_in_upper_half;
    if(is_in_drag_drop_)
    {
      updateRow(row);
      updateRow(row - 1);
      updateRow(row + 1);
    }
  }
  updateMouseHoverRow(row);
}

bool DeployerListView::enableDragReorder() const
{
  return enable_drag_reorder_;
}

void DeployerListView::setEnableDragReorder(bool enabled)
{
  enable_drag_reorder_ = enabled;
}
