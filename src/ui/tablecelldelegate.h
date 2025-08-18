/*!
 * \file tablecelldelegate.h
 * \brief Header for the TableCellDelegate class.
 */

#pragma once

#include "modlistproxymodel.h"
#include "modlistview.h"
#include <QStyledItemDelegate>


/*!
 * \brief Paints a cell containing text or an icon in a ModListView.
 */
class TableCellDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  /*!
   * \brief Constructor.
   * \param parent Parent of this object.
   * \param proxy Proxy model used, or nullptr if non is used.
   */
  explicit TableCellDelegate(QSortFilterProxyModel* proxy, QObject* parent);

  /*!
   * \brief Paints the cells background and text or icon.
   *
   * Uses alternating row colors and highlight / mouse hover colors,
   * depending on the selection status of the cell.
   * \param painter Painter used to draw.
   * \param option Style options.
   * \param view_index The target views index.
   */
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& view_index) const override;

  bool editorEvent(QEvent* event, QAbstractItemModel* model,
                      const QStyleOptionViewItem& option,
                      const QModelIndex& index) override;

protected:
  /*! \brief Proxy model used to sort or filter the underlying model. */
  QSortFilterProxyModel* proxy_model_ = nullptr;
  /*! \brief Convenience pointer to parent view. Points to the same address as this->parent. */
  ModListView* parent_view_;
  int indentation_shift;
};
