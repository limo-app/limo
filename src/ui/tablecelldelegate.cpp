#include "tablecelldelegate.h"
#include "modlistmodel.h"
#include <QApplication>
#include <QPainter>


TableCellDelegate::TableCellDelegate(QSortFilterProxyModel* proxy, QObject* parent) :
  QStyledItemDelegate{ parent }, proxy_model_(proxy),
  parent_view_(static_cast<ModListView*>(parent))
{}

void TableCellDelegate::paint(QPainter* painter,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& view_index) const
{
  auto model_index = proxy_model_ == nullptr ? view_index : proxy_model_->mapToSource(view_index);
  QStyleOption cell;
  QRect rect = option.rect;
  cell.rect = rect;
  const bool is_even_row = view_index.row() % 2 == 0;
  const int mouse_row = parent_view_->getHoverRow();
  const bool row_is_selected =
    parent_view_->selectionModel()->rowIntersectsSelection(view_index.row());
  if(row_is_selected)
  {
    cell.palette.setBrush(
      QPalette::Base,
      option.palette.color(parent_view_->hasFocus() ? QPalette::Active : QPalette::Inactive,
                           QPalette::Highlight));
  }
  else if(mouse_row == view_index.row() && !parent_view_->isInDragDrop())
  {
    const float color_ratio = 0.8;
    auto hl_color = option.palette.color(QPalette::Highlight);
    auto bg_color = option.palette.color(is_even_row ? QPalette::Base : QPalette::AlternateBase);
    auto mix_color = hl_color;
    mix_color.setRed(hl_color.red() * (1 - color_ratio) + bg_color.red() * color_ratio);
    mix_color.setGreen(hl_color.green() * (1 - color_ratio) + bg_color.green() * color_ratio);
    mix_color.setBlue(hl_color.blue() * (1 - color_ratio) + bg_color.blue() * color_ratio);
    cell.palette.setBrush(QPalette::Base, QBrush(mix_color));
  }
  else if(!is_even_row)
    cell.palette.setBrush(QPalette::Base, option.palette.alternateBase());
  QPixmap map(rect.width(), rect.height());
  map.fill(cell.palette.color(QPalette::Base));
  painter->drawPixmap(rect, map);
  auto icon_var = model_index.data(ModListModel::icon_role);
  if(icon_var.isNull())
  {
    auto fg_brush_var = view_index.data(Qt::ForegroundRole);
    auto old_pen = painter->pen();
    if(row_is_selected)
      painter->setPen(option.palette.color(QPalette::HighlightedText));
    else if(!fg_brush_var.isNull())
      painter->setPen(fg_brush_var.value<QBrush>().color());
    auto icon_rect = rect;
    icon_rect.setLeft(rect.left() + 3);
    icon_rect.setBottom(rect.bottom() - 1);
    QApplication::style()->drawItemText(painter,
                                        icon_rect,
                                        Qt::AlignLeft | Qt::AlignVCenter,
                                        option.palette,
                                        true,
                                        model_index.data().toString());
    painter->setPen(old_pen);
  }
  else
  {
    auto icon = icon_var.value<QIcon>();
    const int icon_width = 16;
    const int icon_height = 16;
    const QPixmap icon_map = icon.pixmap(icon_width, icon_height);
    const auto center = rect.center();
    painter->drawPixmap(center.x() - icon_width / 2 + 1, center.y() - icon_height / 2, icon_map);
  }
  if(parent_view_->isInDragDrop())
  {
    if(parent_view_->mouseInUpperHalfOfRow() && mouse_row == view_index.row() ||
       !parent_view_->mouseInUpperHalfOfRow() && mouse_row + 1 == view_index.row())
      painter->drawLine(rect.topLeft(), rect.topRight());
    else if(!parent_view_->mouseInUpperHalfOfRow() && mouse_row == view_index.row() ||
            parent_view_->mouseInUpperHalfOfRow() && mouse_row - 1 == view_index.row())
      painter->drawLine(rect.bottomLeft(), rect.bottomRight());
  }
  if(!parent_view_->selectionModel()->rowIntersectsSelection(view_index.row()) &&
     parent_view_->selectionModel()->currentIndex().row() == view_index.row())
  {
    QStyleOptionFocusRect indicator;
    indicator.rect = option.rect;
    QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &indicator, painter);
  }
}
