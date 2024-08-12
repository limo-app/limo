/*!
 * \file enterapipwdialog.h
 * \brief Header for the EnterApiPwDialog class.
 */

#pragma once

#include <QDialog>


namespace Ui
{
class EnterApiPwDialog;
}

/*!
 * \brief Dialog for entering the password used to encrypt the NexusMods API key.
 */
class EnterApiPwDialog : public QDialog
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
  explicit EnterApiPwDialog(const std::string& cipher,
                            const std::string& nonce,
                            const std::string& tag,
                            QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~EnterApiPwDialog();

  /*!
   * \brief Returns the decrypted API key, if encryption was successful.
   * Else: Returns an empty string.
   * \return The API key.
   */
  std::string getApiKey() const;
  /*!
   * \brief Indicates whether decryption was successful.
   * \return True on success, else false.
   */
  bool wasSuccessful() const;

private slots:
  /*!
   *  \brief Tries to decrypt the API key with the entered password.
   *  On success: Closes the dialog. Else: Indicates failure to the user.
   */
  void on_buttonBox_accepted();
  /*! \brief Closes the dialog. */
  void on_buttonBox_rejected();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::EnterApiPwDialog* ui;
  /*! \brief Cypher text of the API key. */
  std::string cipher_;
  /*! \brief AES-GCM nonce used during encryption. */
  std::string nonce_;
  /*! \brief AES-GCM authorization tag generated during encryption. */
  std::string tag_;
  /*! \brief The decrypted API key, if decryption was successful. */
  std::string api_key_ = "";
  /*! \brief True if decryption was successful, else false. */
  bool success_ = false;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;
};
