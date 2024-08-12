/*!
 * \file addtooldialog.h
 * \brief Header for the AddToolDialog class.
 */

#pragma once

#include <QDialog>

namespace Ui
{
class AddToolDialog;
}

/*!
 * \brief Dialog for adding a new tool.
 */
class AddToolDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddToolDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddToolDialog();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddToolDialog* ui;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

  /*!
   * \brief Updates the enabled state of this dialog's OK button to only be enabled when
   * both a name and a command has been entered.
   */
  void updateOkButton();

public:
  /*! \brief Clears name and command input field. */
  void setupDialog();

private slots:
  /*! \brief Closes the dialog and emits a signal for completion. */
  void on_buttonBox_accepted();
  /*! \brief Only enable the OK button if a name has been entered. */
  void on_name_field_textChanged(const QString& text);
  /*! \brief Only enable the OK button if a command has been entered. */
  void on_command_field_textChanged(const QString& text);

signals:
  /*!
   * \brief Signals dialog completion.
   * \param name Name of the new tool.
   * \param command Command used to run the new tool.
   */
  void toolAdded(QString name, QString command);
};
