/*!
 * \file addbackuptargetdialog.h
 * \brief Header for the AddBackupTargetDialog class.
 */

#pragma once

#include <QDialog>
#include <QFileDialog>


namespace Ui
{
class AddBackupTargetDialog;
}

/*!
 * \brief Dialog for adding new backup targets.
 */
class AddBackupTargetDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddBackupTargetDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddBackupTargetDialog();

  /*!
   * \brief Removes the text from all input fields.
   * \param app_id Application to which the new backup target is to be added.
   */
  void resetDialog(int app_id);

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddBackupTargetDialog* ui;
  /*! \brief File dialog used to select a backup target. */
  std::unique_ptr<QFileDialog> file_dialog_;
  /*! \brief Application to which the new backup target is to be added. */
  int app_id_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

  /*!
   * \brief Updates the Ok button to only be enabled if the target path, name and default
   * backup fields are filled.
   */
  void updateOkButton();
  /*! \brief Verifies if the target path field refers to an existing file or directory. */
  bool pathIsValid();

private slots:
  /*!
   * \brief Calls \ref updateOkButton.
   * \param text Ignored.
   */
  void on_target_name_field_textEdited(const QString& text);
  /*!
   * \brief Calls \ref updateOkButton.
   * \param text Ignored.
   */
  void on_target_path_field_textEdited(const QString& text);
  /*!
   * \brief Calls \ref updateOkButton.
   * \param text Ignored.
   */
  void on_default_backup_field_textEdited(const QString& text);
  /*! \brief Signals dialog completion by emitting \ref backupTargetAdded. */
  void on_buttonBox_accepted();
  /*! \brief Opens a file dialog to pick a target path. */
  void on_path_picker_button_clicked();
  /*!
   * \brief Updates the target path field with the new path.
   * \param path The selected path.
   */
  void onFileDialogAccepted(const QString& path);
  /*!
   * \brief Updates the file mode of file_dialog_ to allow selection of both files and
   * directories.
   * \param path Currently selected item.
   */
  void onFileDialogSelectionChanged(const QString& path);

signals:
  /*!
   * \brief Signals dialog has been accepted.
   * \param app_id Application to which the new backup target is to be added.
   * \param target_name Name of the new backup target.
   * \param target_path Path to the file or directory to be managed.
   * \param default_backup Name of the currently active version of the target.
   * \param first_backup If not empty: Name of the first backup.
   */
  void backupTargetAdded(int app_id,
                         QString target_name,
                         QString target_path,
                         QString default_backup,
                         QString first_backup);
};
