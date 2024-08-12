/*!
 * \file addappdialog.h
 * \brief Header for the AddAppDialog class.
 */

#pragma once

#include "../core/editapplicationinfo.h"
#include <QDialog>


namespace Ui
{
class AddAppDialog;
}

/*!
 * \brief Dialog for creating and editing \ref ModdedApplication "applications".
 */
class AddAppDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddAppDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddAppDialog();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddAppDialog* ui;
  /*! \brief If true: Dialog is used to edit, else: Dialog is used to create. */
  bool edit_mode_ = false;
  /*! \brief Current name of the edited \ref ModdedApplication "application". */
  QString name_;
  /*! \brief Current staging directory path of the edited \ref ModdedApplication "application". */
  QString path_;
  /*! \brief Current command to run the edited \ref ModdedApplication "application". */
  QString command_;
  /*! \brief Id of the edited \ref ModdedApplication "application". */
  int app_id_;
  /*! \brief Path to imported steam applications installation directory. */
  QString steam_install_path_ = "";
  /*! \brief Path to imported steam applications prefix directory. */
  QString steam_prefix_path_ = "";
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

  /*!
   * \brief Set the enabled state of this dialogs OK button.
   * \param state
   */
  void enableOkButton(bool state);
  /*! \brief Checks whether the currently entered path exists. */
  bool pathIsValid();
  /*!
   * \brief Checks whether the currently entered icon path refers to a valid icon file.
   * \param Path to an icon. If this checked instead of ui->icon_field if this is not empty.
   */
  bool iconIsValid(const QString& path = "");

public:
  /*!
   * \brief Initializes this dialog to allow editing of an existing
   * \ref ModdedApplication "application".
   * \param name Current name of the edited \ref ModdedApplication "application".
   * \param app_version Current app app_version.
   * \param path Current staging directory path of the edited
   * \ref ModdedApplication "application".
   * \param command Current command to run the edited \ref ModdedApplication "application".
   * \param app_id Id of the edited \ref ModdedApplication "application".
   */
  void setEditMode(const QString& name,
                   const QString& app_version,
                   const QString& path,
                   const QString& command,
                   const QString& icon_path,
                   int app_id);
  /*!
   *  \brief Initializes this dialog to allow creating a new
   *  \ref ModdedApplication "application".
   */
  void setAddMode();

private slots:
  /*! \brief Shows a file dialog for the staging directory path. */
  void on_file_picker_button_clicked();
  /*! \brief Only enable the OK button if a name has been entered. */
  void on_name_field_textChanged(const QString& text);
  /*! \brief Only enable the OK button if a valid staging directory path has been entered. */
  void on_path_field_textChanged(const QString& text);
  /*! \brief Closes the dialog and emits a signal for completion. */
  void on_buttonBox_accepted();
  /*! \brief Opens a dialog to import currently installed steam app. */
  void on_import_button_clicked();
  /*!
   * \brief Called when the import steam application dialog has been completed.
   * \param name Name of the imported application.
   * \param app_id Steam app_id of the imported application.
   * \param install_dir Name of the directory under steamapps which contains the
   * new applications files.
   * \param prefix_path Path to the applications Proton prefix, or empty if none exists.
   * \param icon_path Path to the applications icon.
   */
  void onApplicationImported(QString name,
                             QString app_id,
                             QString install_dir,
                             QString prefix_path,
                             QString icon_path);
  /*!
   * \brief Updates the staging directory path to given path.
   * \param path The new path.
   */
  void onFileDialogAccepted(const QString& path);
  /*! \brief Called when icon path picker button is clicked. */
  void on_icon_picker_button_clicked();
  /*!
   * \brief Updates the icon path to the given path if the given path refers to a valid icon.
   * \param path The new path.
   */
  void onIconPathDialogComplete(const QString& path);

signals:
  /*!
   * \brief Signals completion of the dialog in add mode.
   * \param info Contains all data entered in the dialog.
   */
  void applicationAdded(EditApplicationInfo edit_app_info);
  /*!
   * \brief Signals completion of the dialog in edit mode.
   * \param info Contains all data entered in the dialog.
   * \param app_id Id of the edited \ref ModdedApplication "application".
   */
  void applicationEdited(EditApplicationInfo edit_app_info, int app_id);
};
