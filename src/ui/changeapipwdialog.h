/*!
 * \file addapikeydialog.h
 * \brief Header for the AddApiKeyDialog class.
 */
#pragma once

#include <QDialog>


namespace Ui
{
class ChangeApiPwDialog;
}

/*!
 * \brief Dialog used for changing the current password used to encrypt the NexusMods API key.
 */
class ChangeApiPwDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the dialog with data needed for decryption.
   * \param cipher Cypher text of the API key.
   * \param nonce AES-GCM nonce used during encryption.
   * \param tag AES-GCM authorization tag generated during encryption.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit ChangeApiPwDialog(bool uses_default_pw,
                             const std::string& cipher = "",
                             const std::string& nonce = "",
                             const std::string& tag = "",
                             QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~ChangeApiPwDialog();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::ChangeApiPwDialog* ui;
  /*! \brief Cypher text of the API key. */
  std::string cipher_;
  /*! \brief AES-GCM nonce used during encryption. */
  std::string nonce_;
  /*! \brief AES-GCM authorization tag generated during encryption. */
  std::string tag_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;
  /*! \brief If true: Key is encrypted using cryptography::default_key */
  bool uses_default_pw_;

private slots:
  /*!
   * \brief Disables/ enables the OK button, depending on if the entered passwords match.
   * \param is_valid True if both passwords match.
   */
  void onPasswordValidityChanged(bool is_valid);
  /*!
   *  \brief Tries to decrypt the API key with the old password. If that fails: Shows an error.
   *  On success: Encrypts the key with the new password and closes the dialog.
   */
  void on_button_box_accepted();
  /*! \brief Closes the dialog */
  void on_button_box_rejected();

signals:
  /*!
   * \brief Signals that the NexusMods api key is to be encrypted with a new password.
   * \param cipher Cypher text of the API key.
   * \param nonce AES-GCM nonce used during encryption.
   * \param tag AES-GCM authorization tag generated during encryption.
   * \param uses_default_pw If true: User set no password.
   */
  void keyEncryptionUpdated(std::string cipher,
                            std::string nonce,
                            std::string tag,
                            bool default_pw);
};
