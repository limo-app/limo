/*!
 * \file modlistview.h
 * \brief Header for the ModListView class.
 */

#pragma once

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QTableView>
#include <QWidget>
#include <QtCore>
#include <set>


/*!
 * \brief Displays mod data in the form of a mod list using a ModListModel.
 */
class ModListView : public QTableView
{
  Q_OBJECT
public:
  /*!
   * \brief Simply calls QTableview's constructor with parent as argument.
   * \param parent The parent widget for this widget.
   */
  explicit ModListView(QWidget* parent = nullptr);

  /*!
   * \brief Returns true iff buttons in this view will react to mouse inputs.
   * \return The button status.
   */
  bool enableButtons() const;
  /*!
   * \brief Sets whether buttons in this view will react to mouse inputs.
   * \param enabled Button will react iff this is true.
   */
  void setEnableButtons(bool enabled);
  /*!
   * \brief Returns the row currently under the mouse, or -1 if no row is under the mouse.
   * \return The row.
   */
  int getHoverRow() const;
  /*!
   * \brief Returns true iff an item is currently being moved by drag and drop.
   * \return True while in drag drop mode.
   */
  bool isInDragDrop() const;
  /*!
   * \brief Returns true iff mouse is currently in the upper half of a row.
   * \return True iff mouse is currently in the upper half of a row.
   */
  bool mouseInUpperHalfOfRow() const;
  /*!
   * \brief Returns the number of currently selected rows.
   * \return The number of rows.
   */
  int getNumSelectedRows() const;
  /*!
   * \brief Returns a vector of selected mod ids.
   * \return The vector.
   */
  std::vector<int> getSelectedModIds() const;
  /*!
   * \brief Returns one index for every selected row. Columns will be set to 0.
   * \return The list.
   */
  QModelIndexList getSelectedRowIndices() const;

protected:
  /*! \brief Last row on which a mouse button has been pressed. */
  int mouse_down_row_ = -1;
  /*! \brief Last row over which the cursor hovered. */
  int mouse_hover_row_ = -1;
  /*! \brief Determines if buttons react to inputs. */
  bool enable_buttons_ = true;
  /*! \brief Indicates if an item is currently being moved by drag and drop. */
  bool is_in_drag_drop_ = false;
  /*! \brief Stores if mouse is currently in the upper half of a row. */
  bool mouse_in_upper_half_of_row_ = false;

  /*!
   * \brief If dropped item was a file or a list of files, emit \ref modAdded.
   * \param event The drop event.
   */
  void dropEvent(QDropEvent* event) override;
  /*!
   * \brief Enables drag enter events.
   * \param The drag enter event.
   */
  void dragEnterEvent(QDragEnterEvent* event) override;
  /*!
   * \brief Enables drag leave events.
   * \param The drag leave event.
   */
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  /*!
   * \brief Enables drag move events.
   * \param The drag move event.
   */
  void dragMoveEvent(QDragMoveEvent* event) override;
  /*!
   * \brief Gets called when the mouse has been pressed while in this widget.
   *
   * Highlights the currently selected row and forwards edit events.
   * \param event The source event.
   */
  void mousePressEvent(QMouseEvent* event) override;
  /*!
   * \brief Gets called when the mouse has been released while in this widget.
   *
   * If the mouse was released on column 0 and the same row on which it was pressed:
   * Emits \ref modRemoved.
   * \param event The source event.
   */
  void mouseReleaseEvent(QMouseEvent* event) override;
  /*!
   * \brief Gets called when the mouse has been moved while in this widget.
   *
   * Highlights the row currently under the cursor.
   * \param event The source event.
   */
  void mouseMoveEvent(QMouseEvent* event) override;
  /*!
   * \brief Gets called when a double click has been performed in this widget.
   *
   * Calls the respective editor for mod name and mod version name.
   * \param event The source event.
   */
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  /*!
   * \brief Gets called when the cursor leaves this widget. Resets mouse over highlighting.
   * \param event The source event.
   */
  void leaveEvent(QEvent* event) override;
  /*!
   * \brief Gets called when this widget loses focus. Changes highlight color to inactive.
   * \param event The source event.
   */
  void focusOutEvent(QFocusEvent* event) override;
  /*!
   * \brief Gets called when this widget gains focus. Changes highlight color to active.
   * \param event The source event.
   */
  void focusInEvent(QFocusEvent* event) override;
  /*!
   * \brief Disables moving the cursor with keyboard inputs
   * \param cursorAction Action taken.
   * \param modifiers Key modifiers.
   * \return The current model index.
   */
  QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
                         Qt::KeyboardModifiers modifiers) override;
  /*!
   * \brief Checks if given row index refers to an existing row.
   * \param row Index to be checked.
   * \return True if index is valid.
   */
  bool rowIndexIsValid(int row) const;
  /*!
   * \brief Sets mouse_hover_row_ to the given row and updates the view accordingly.
   * \param row New row under the mouse.
   */
  void updateMouseHoverRow(int row);
  /*!
   * \brief Sets mouse_down_row_ to the given row and updates the view accordingly.
   * \param row Row which has been clicked.
   */
  void updateMouseDownRow(int row);
  /*!
   * \brief Repaints the given rows
   * \param row Row to repaint.
   */
  void updateRow(int row);

signals:
  /*!
   * \brief Signals files have been dropped into this widget.
   * \param path Paths to the dropped files.
   */
  void modAdded(QList<QUrl> path);
  /*!
   *  \brief Signals that a mods activation status has been changed.
   *  \param mod_id Target mod.
   *  \param status New mod status.
   */
  void modStatusChanged(int mod_id, bool status);
  /*!
   *  \brief Signals that a mod has been removed.
   *  \param mod_id Id of the mod which is to be removed.
   *  \param name Name of the mod which is to be removed.
   */
  void modRemoved(int mod_id, QString name);
};
