/*!
 * \file tabletoolbutton.h
 * \brief Header for the TableToolButton class.
 */

#pragma once

#include <QObject>
#include <QToolButton>


/*!
 * \brief QToolButton derivative for use in a QTableWidget cell. This button knows it's
 * position in the table.
 */
class TableToolButton : public QToolButton
{
  Q_OBJECT
public:
  /*!
   * \brief Since QTableWidget takes ownership of it's cell widgets, it is not necessary
   * to take a parent object or delete this object manually.
   * \param row Row in a QTableWidget which contains this button.
   */
  TableToolButton(int row);

private:
  /*! \brief Row of the QTableWidget which contains this button. */
  const int row_;

public slots:
  /*! \brief Called when the Run tool action is clicked. */
  void onRunClicked();
  /*! \brief Called then the Remove tool action is clicked. */
  void onRemoveClicked();

signals:
  /*!
   * \brief Signals the Run tool action has been clicked.
   * \param row Row containing this button.
   */
  void clickedRunAt(int row);
  /*!
   * \brief Signals the Remove tool action has been clicked.
   * \param row Row containing this button.
   */
  void clickedRemoveAt(int row);
};
