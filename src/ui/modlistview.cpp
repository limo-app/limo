#include "modlistview.h"
#include "modlistmodel.h"
#include "qmodelindexutils.h"
#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>
#include <QAbstractItemModel>
#include <ranges>

namespace str = std::ranges;


ModListView::ModListView(QWidget* parent) : QTreeView(parent)
{
  setMouseTracking(true);
  QFile styleFile(":/styles/tablecellstyle.qss");
  styleFile.open(QFile::ReadOnly);
  setStyleSheet(styleFile.readAll());
}

void ModListView::dropEvent(QDropEvent* event)
{
  const QMimeData* mime_data = event->mimeData();
  if(mime_data->hasUrls())
    emit modAdded(mime_data->urls());
}

void ModListView::dragEnterEvent(QDragEnterEvent* event)
{
  event->acceptProposedAction();
}

void ModListView::dragLeaveEvent(QDragLeaveEvent* event)
{
  event->accept();
}

void ModListView::dragMoveEvent(QDragMoveEvent* event)
{
  event->acceptProposedAction();
}

void ModListView::mousePressEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int event_col = index.column();
  const auto selection = QItemSelection(model()->index(event_row, 0),
                                        model()->index(event_row, model()->columnCount() - 1));
  const auto modifiers = QGuiApplication::keyboardModifiers();
  if(modifiers.testFlag(Qt::ControlModifier) && !modifiers.testFlag(Qt::ShiftModifier) &&
     event_row != -1)
  {
    selectionModel()->select(selection, QItemSelectionModel::Toggle);
    selectionModel()->setCurrentIndex(model()->index(event_row, event_col),
                                      QItemSelectionModel::NoUpdate);
  }
  else if(modifiers.testFlag(Qt::ShiftModifier) && !modifiers.testFlag(Qt::ControlModifier) &&
          event_row != -1)
  {
    const int index_row = selectionModel()->currentIndex().row();
    if(index_row >= 0 && index_row != event_row)
    {
      const int first_row = index_row < event_row ? index_row : event_row;
      const int last_row = index_row < event_row ? event_row : index_row;
      const auto indices = selectionModel()->selection().indexes();
      for(const auto& index : indices)
        selectionModel()->select(index, QItemSelectionModel::Deselect);
      auto new_selection = QItemSelection(model()->index(first_row, 0),
                                          model()->index(last_row, model()->columnCount() - 1));
      selectionModel()->select(new_selection, QItemSelectionModel::Select);
    }
  }
  else
  {
    const auto indices = selectionModel()->selection().indexes();
    if(indices.size() <= model()->columnCount() || event->button() == Qt::LeftButton)
    {
      selectionModel()->clear();
      for(const auto& index : indices)
        updateRow(index);
      selectionModel()->setCurrentIndex(model()->index(event_row, event_col),
                                        QItemSelectionModel::Select);
      selectionModel()->select(selection, QItemSelectionModel::Select);
    }
  }
  updateMouseDownRow(index);
}

void ModListView::mouseReleaseEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int event_col = index.column();
  if(event_row != mouse_down_.row())
    updateMouseHoverRow(QModelIndex());
  else if(event_col == 0 && event_row > -1 && event_row < model()->rowCount())
  {
    if(enable_buttons_)
      emit modRemoved(model()->data(index, ModListModel::mod_id_role).toInt(),
                      model()->data(index, ModListModel::mod_name_role).toString());
  }
  else if(event_col == ModListModel::version_col && event->button() == Qt::LeftButton &&
          columnViewportPosition(event_col) + columnWidth(event_col) - 18 < event->x() &&
          static_cast<ModListProxyModel*>(model())->isEditable())
    edit(model()->index(event_row, event_col));
}

bool ModListView::rowIndexIsValid(int row) const
{
  return row > -1 && row < model()->rowCount();
}

void ModListView::updateMouseHoverRow(QModelIndex index)
{
  if(!sameRow(mouse_hover_, index))
  {
    updateRow(index);
    updateRow(mouse_hover_);
    mouse_hover_ = index;
  }
}

void ModListView::updateMouseDownRow(QModelIndex index)
{
  if(mouse_down_ != index)
  {
    updateRow(index);
    updateRow(mouse_down_);
    mouse_down_ = index;
  }
}

void ModListView::updateRow(QModelIndex index)
{
  if(index.isValid())
  {
    for(int col = 0; col < model()->columnCount(); col++)
      update(model()->index(index.row(), col,index.parent()));
  }
}

bool ModListView::isInDragDrop() const
{
  return is_in_drag_drop_;
}

int ModListView::getMouseRegion() const
{
  return mouse_row_region;
}

int ModListView::getNumSelectedRows() const
{
  return selectionModel()->selection().indexes().size() / model()->columnCount();
}

std::vector<int> ModListView::getSelectedModIds() const
{
  const auto indices = selectionModel()->selectedIndexes();
  std::vector<int> mod_ids;
  for(const auto& index : indices)
  {
    const int mod_id = index.data(ModListModel::mod_id_role).toInt();
    if(str::find(mod_ids, mod_id) == mod_ids.end())
      mod_ids.push_back(mod_id);
  }
  return mod_ids;
}

QModelIndexList ModListView::getSelectedRowIndices() const
{
  const auto all_indices = selectedIndexes();
  QModelIndexList row_indices;
  for(int i = 0; i < all_indices.size(); i += model()->columnCount())
  {
    row_indices.append(all_indices[i]);
  }
  return row_indices;
}

QModelIndex ModListView::getHoverRow() const
{
  return mouse_hover_;
}

void ModListView::mouseMoveEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  updateMouseHoverRow(index);
}

void ModListView::mouseDoubleClickEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int event_col = index.column();
  if(event->button() == Qt::LeftButton && event_row == mouse_down_.row() &&
     (event_col == ModListModel::name_col ||
      event_col == ModListModel::version_col &&
        columnViewportPosition(event_col) + columnWidth(event_col) - 18 >= event->x()) &&
     static_cast<ModListProxyModel*>(model())->isEditable())
    edit(index);
}

void ModListView::leaveEvent(QEvent* event)
{
  if(rect().contains(mapFromGlobal(QCursor::pos())) || !mouse_hover_.isValid() ||
     mouse_hover_.row() == currentIndex().row())
    return;
  updateMouseHoverRow(QModelIndex());
  QTreeView::leaveEvent(event);
}

void ModListView::focusOutEvent(QFocusEvent* event)
{
  if(QGuiApplication::mouseButtons().testFlag(Qt::RightButton) &&
     rect().contains(mapFromGlobal(QCursor::pos())))
    return;
  QTreeView::focusOutEvent(event);
}

void ModListView::focusInEvent(QFocusEvent* event)
{
  QTreeView::focusInEvent(event);
}

QModelIndex ModListView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
  return currentIndex();
}

bool ModListView::enableButtons() const
{
  return enable_buttons_;
}

void ModListView::setEnableButtons(bool enabled)
{
  enable_buttons_ = enabled;
}
