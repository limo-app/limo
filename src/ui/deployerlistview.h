/*!
 * \file deployerlistview.h
 * \brief Header for the DeployerListView class.
 */

#pragma once

#include "modlistview.h"


/*!
 * \brief Displays mod data either in the form of a deployer list using a DeployerListModel.
 */
class DeployerListView : public ModListView
{
  Q_OBJECT
public:
  /*!
   * \brief Simply calls ModListView's constructor with parent as argument.
   * \param parent The parent widget for this widget.
   */
  explicit DeployerListView(QWidget* parent = nullptr);

  /*! \brief Checks if drag and drop is enabled. */
  bool enableDragReorder() const;
  /*!
   * \brief Enables or disables drag and drop support.
   * \param enabled Mew state.
   */
  void setEnableDragReorder(bool enabled);
  void setModel(QAbstractItemModel* model) override;

protected:
  /*!
   * \brief Gets called when the mouse has been pressed while in this widget.
   *
   * Highlights the currently selected row and implements drag and drop.
   * \param event The source event.
   */
  void mousePressEvent(QMouseEvent* event) override;
  /*!
   * \brief Gets called when the mouse has been released while in this widget.
   *
   * Implements drag and drop and emits \ref modMoved. Resets cursor style to default.
   * If the mouse was released on column 0 and the same row on which it was pressed:
   * Emits \ref modStatusChanged.
   * \param event The source event.
   */
  void mouseReleaseEvent(QMouseEvent* event) override;
  /*!
   * \brief Gets called when the mouse has been moved while in this widget.
   *
   * Highlights the row currently under the cursor. Sets cursor style to a drag
   * and drop design if left button is being held.
   * \param event The source event.
   */
  void mouseMoveEvent(QMouseEvent* event) override;
  void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
  /*! \brief Toggles drag and drop support. */
  bool enable_drag_reorder_ = false;
  void onExpanded(const QModelIndex &);
  void onCollapsed(const QModelIndex &);
  void expandSeparators(const QModelIndex & = QModelIndex());

signals:
  /*!
   * \brief Signals a mod has been dragged to a new position.
   * \param Original mod row.
   * \param New mod row.
   */
  void modMoved();
};
