/*!
 * \file settingsdialog.h
 * \brief Header for the SettingsDialog class.
 */

#pragma once

#include "loot/api.h"
#include <QDialog>
#include <QIcon>
#include <map>


namespace Ui
{
class SettingsDialog;
}

/*!
 * \brief Dialog for changing various application settings.
 */
class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   * \param is_flatpak Whether or not this application is running as a flatpak.
   */
  explicit SettingsDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~SettingsDialog();

  /*! \brief Initializes the dialog with the settings stored on disc. */
  void init();

  /*!
   * \brief Returns true if the ask when removing a mod option has been selected.
   * \return The selection.
   */
  bool askRemoveMod() const;
  /*!
   * \brief Returns true if the ask when removing a mod from deployer option has been selected.
   * \return The selection.
   */
  bool askRemoveFromDeployer() const;
  /*!
   * \brief Returns true if the ask when removing a profile option has been selected.
   * \return The selection.
   */
  bool askRemoveProfile() const;
  /*!
   * \brief Returns true if the deploy for all option has been selected.
   * \return The selection.
   */
  bool deployAll() const;
  /*!
   * \brief Returns true if the log on error option has been selected.
   * \return The selection.
   */
  bool logOnError() const;
  /*!
   * \brief Returns true if the log on warning option has been selected.
   * \return The selection.
   */
  bool logOnWarning() const;
  /*!
   * \brief Returns true if the ask when removing a backup target option has been selected.
   * \return The selection.
   */
  bool askRemoveBackupTarget() const;
  /*!
   * \brief Returns true if the ask when removing a backup option has been selected.
   * \return The selection.
   */
  bool askRemoveBackup() const;
  /*!
   * \brief Returns true if the ask when removing tool option has been selected.
   * \return The selection.
   */
  bool askRemoveTool() const;
  /*!
   * \brief Reads the key cipher, nonce, tag and the is_default flag for the Nexus API key from
   * the settings file.
   * \return The key cipher, nonce, tag and the is_default flag. An empty optional if no key exists
   * in the settings.
   */
  static std::optional<std::tuple<std::string, std::string, std::string, bool>>
  getNexusApiKeyDetails();

private slots:
  /*!
   * \brief Updates the settings on disc with the selection made in this dialog.
   * Emits \ref settingsDialogAccepted.
   */
  void on_buttonBox_accepted();
  /*! \brief Opens a AddApiKeyDialog to add a new api key. */
  void on_set_api_key_button_clicked();
  /*! \brief Initializes and executes a ChangeApiPwDialog. */
  void on_change_api_pw_button_clicked();
  /*! \brief Toggles visibility of the API key. */
  void on_show_api_key_button_clicked();

public slots:
  /*!
   * \brief Writes details about the encrypted Nexus API key to the settings file.
   * \param cipher Cypher text of the API key.
   * \param nonce AES-GCM nonce used during encryption.
   * \param tag AES-GCM authorization tag generated during encryption.
   * \param uses_default_pw If true: User set no password.
   */
  void setNexusCryptographyFields(const std::string& cipher,
                                  const std::string& nonce,
                                  const std::string& tag,
                                  bool uses_default_pw);

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::SettingsDialog* ui;
  /*! \brief True if the ask when removing a mod option has been selected. */
  bool ask_remove_mod_ = true;
  /*! \brief True if the ask when removing a mod from deployer option has been selected. */
  bool ask_remove_from_deployer_ = true;
  /*! \brief True if the ask when removing a profile option has been selected. */
  bool ask_remove_profile_ = true;
  /*! \brief True if the deploy for all option has been selected. */
  bool deploy_all_ = true;
  /*! \brief True if the log on error option has been selected. */
  bool log_on_error_ = true;
  /*! \brief True if the log on warning option has been selected. */
  bool log_on_warning_ = true;
  /*! \brief True if the ask when removing a backup target option has been selected. */
  bool ask_remove_backup_target_ = true;
  /*! \brief True if the ask when removing a backup option has been selected. */
  bool ask_remove_backup_ = true;
  /*! \brief True if the ask when removing a tool option has been selected. */
  bool ask_remove_tool_ = true;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;
  /*! \brief Icon used to indicate that the password is to be shown. */
  const QIcon show_icon = QIcon::fromTheme("view-visible");
  /*! \brief Icon used to indicate that the password is to be hidden. */
  const QIcon hide_icon = QIcon::fromTheme("view-hidden");
  /*! \brief Text shown instead of an API key when the visibility is set to hidden. */
  const QString api_key_hidden_string = "API Key: ***";

signals:
  /*! \brief Signals dialog completion. */
  void settingsDialogAccepted();
};
