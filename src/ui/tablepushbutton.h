/*!
 * \file tablepushbutton.h
 * \brief Header for the TablePushButton class.
 */

#pragma once

#include <QPushButton>


/*!
 * \brief QPushButton derivative for use in a QTableWidget cell. This button knows it's
 * position in the table.
 */
class TablePushButton : public QPushButton
{
  Q_OBJECT
public:
  /*!
   * \brief Constructs a new button for the given row and column.
   * \param row Target row.
   * \param col Target column.
   */
  TablePushButton(int row, int col);

private:
  /*! \brief Row of the QTableWidget which contains this button. */
  const int row_;
  /*! \brief Column of the QTableWidget which contains this button. */
  const int col_;

private slots:
  /*! \brief Called when the button is clicked. Emits clickedAt. */
  void onClickedAt();

signals:
  /*!
   * \brief Signals button has been clicked at a specific table position.
   * \param row Table row.
   * \param col Table column.
   */
  void clickedAt(int row, int col);
};
