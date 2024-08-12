/*!
 * \file movemoddialog.h
 * \brief Header for the MoveModDialog class.
 */

#pragma once

#include <QDialog>


namespace Ui
{
class MoveModDialog;
}

/*!
 * \brief Dialog used to move a mod to a new position in a load order.
 */
class MoveModDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param name The target mod's name.
   * \param source The target mod's current position.
   * \param num_mods Number of mods in the load order.
   * \param parent Parent of this widget.
   */
  explicit MoveModDialog(QString name, int source, int num_mods, QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~MoveModDialog();

private slots:
  /*! \brief Closes the dialog and emits a signal for completion. */
  void on_buttonBox_accepted();
  /*!
   * \brief Enables the OK button only if the new text is an int between 1 and the number
   * of mods in the load order.
   * \param new_text Newly entered text.
   */
  void on_target_field_textEdited(const QString& new_text);

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::MoveModDialog* ui;
  /*! \brief Original position of target mod in the load order. */
  int source_;

signals:
  /*!
   * \brief Signals that a mod has been moved.
   * \param source Original position in the load order.
   * \param target New position in the load order.
   */
  void modMovedTo(int source, int target);
};
