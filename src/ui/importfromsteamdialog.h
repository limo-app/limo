/*!
 * \file importfromsteamdialog.h
 * \brief Header for the ImportFromSteamDialog class.
 */

#pragma once

#include <QDialog>
#include <filesystem>


namespace Ui
{
class ImportFromSteamDialog;
}

class ImportFromSteamDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   * \param is_flatpak Whether or not this application is running as a flatpak.
   */
  explicit ImportFromSteamDialog(bool is_flatpak, QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~ImportFromSteamDialog();
  /*! \brief Initializes the dialog. */
  void init();

private:
  /*! \brief Contains auto-generated ui elements. */
  Ui::ImportFromSteamDialog* ui;
  /*! \brief Name of the file containing all installed steam apps. */
  const std::string library_file_name_ = "libraryfolders.vdf";
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;
  /*! \brief Whether or not this application is running as a flatpak. */
  bool is_flatpak_;

  /*!
   * \brief Checks if given path contains the library file.
   * \param path Path to be checked.
   * \return True if path contains a file named library_file_name, else False.
   */
  bool pathIsValid(std::filesystem::path path) const;
  /*!
   * \brief Updates ui->app_table with all apps listed in the library file.
   * \param steam_dir Path to steam installation.
   */
  void updateTable(std::filesystem::path steam_dir);
  /*!
   * \brief Adds a row to ui->app_table containing information about the app pertaining
   * the given app_id.
   * \param app_id Target app_id.
   * \param path Path to the apps library folder.
   * \return True if a new row has been added.
   */
  bool addTableRow(std::string app_id, std::filesystem::path path);
  /*!
   * \brief Shows an error in a QMessageBox with given title and message.
   * \param title Title of the error box.
   * \param message Error message to be displayed.
   */
  void showError(QString title, QString message);

private slots:
  /*! \brief Opens a file dialog to chose the steam path. */
  void on_pick_path_button_clicked();
  /*! \brief Closes the dialog and emits a signal for completion. */
  void on_buttonBox_accepted();
  /*! \brief Updates ui->app_table with information from the selected directory. */
  void on_path_field_editingFinished();
  /*!
   * \brief Called when the text in ui->search_field has been edited by the user.
   * \param new_text The newly entered text.
   */
  void on_search_field_textEdited(const QString& new_text);

signals:
  /*!
   * \brief Signals completion of the dialog.
   * \param name Name of the imported application.
   * \param app_id Steam app_id of the imported application.
   * \param install_dir Name of the directory under steamapps which contains the
   * new applications files.
   * \param prefix_path Path to the applications Proton prefix, or empty if none exists.
   * \param icon_path Path to the applications icon.
   */
  void applicationImported(QString name,
                           QString app_id,
                           QString install_dir,
                           QString prefix_path,
                           QString icon_path);
};
