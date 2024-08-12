/*!
 * \file addapikeydialog.h
 * \brief Header for the AddApiKeyDialog class.
 */

#pragma once

#include <QDialog>


namespace Ui
{
class AddApiKeyDialog;
}

/*!
 * \brief Dialog used to adding a new NexusMods API key and setting an encryption password.
 */
class AddApiKeyDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddApiKeyDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddApiKeyDialog();

  /*!
   * \brief Returns the API key entered in the dialog.
   * \return The API key.
   */
  QString getApiKey() const;
  /*!
   * \brief Returns the password entered in the dialog.
   * \return The password.
   */
  QString getPassword() const;

private slots:
  /*! \brief Closes the dialog */
  void on_buttonBox_rejected();
  /*! \brief Closes the dialog */
  void on_buttonBox_accepted();
  /*!
   * \brief Disables/ enables the OK button, depending on if the entered passwords match.
   * \param is_valid True if both passwords match.
   */
  void onPasswordValidityChanged(bool is_valid);

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddApiKeyDialog* ui;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;
};
