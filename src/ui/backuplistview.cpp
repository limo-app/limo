#include "backuplistview.h"
#include "backuplistmodel.h"
#include "qmodelindexutils.h"


BackupListView::BackupListView(QWidget* parent) : ModListView(parent) {}

void BackupListView::mousePressEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int event_col = index.column();
  selectionModel()->setCurrentIndex(model()->index(event_row, event_col),
                                    QItemSelectionModel::SelectCurrent);
  updateMouseDownRow(index);
}

void BackupListView::mouseReleaseEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int event_col = index.column();
  const bool is_valid_row =
    !(event_row == index.data(BackupListModel::num_targets_role).toInt() || event_row == -1);
  if(event_row != mouse_down_.row() || mouse_down_.parent() != index.parent())
    updateMouseHoverRow(QModelIndex());
  else if(event_col == BackupListModel::action_col && event_row > -1 &&
          event_row < model()->rowCount() && enable_buttons_)
  {
    if(event_row < model()->data(index, BackupListModel::num_targets_role).toInt())
    {
      QString name = model()->data(index, BackupListModel::target_name_role).toString();
      emit backupTargetRemoved(event_row, name);
    }
    else
      emit addBackupTargetClicked();
  }
  else if(is_valid_row && event_col == BackupListModel::backup_col &&
          event->button() == Qt::LeftButton &&
          columnViewportPosition(event_col) + columnWidth(event_col) - 18 < event->x() &&
          static_cast<BackupListModel*>(model())->isEditable())
    edit(model()->index(event_row, event_col));
}

void BackupListView::mouseDoubleClickEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int event_col = index.column();
  const bool is_valid_row =
    !(event_row == index.data(BackupListModel::num_targets_role).toInt() || event_row == -1);
  if(sameRow(index, mouse_down_) && is_valid_row && event->button() == Qt::LeftButton &&
     (event_col == BackupListModel::target_col ||
      event_col == BackupListModel::backup_col &&
        columnViewportPosition(event_col) + columnWidth(event_col) - 18 >= event->x()) &&
     static_cast<BackupListModel*>(model())->isEditable())
    edit(model()->index(event_row, event_col));
}
