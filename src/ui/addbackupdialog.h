/*!
 * \file addbackupdialog.h
 * \brief Header for the AddBackupDialog class.
 */

#pragma once

#include <QDialog>


namespace Ui
{
class AddBackupDialog;
}

/*!
 * \brief Dialog for adding new backups.
 */
class AddBackupDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddBackupDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddBackupDialog();

  /*!
   * \brief Initializes this dialog with data needed for backup creation.
   * \param Application to which the new backup is to be added.
   * \param target_id Id of the target for which to create a new backup.
   * \param target_name Name of the target for which to create a new backup.
   * \param existing_backups List of all existing backup names for the selected target.
   */
  void setupDialog(int app_id,
                   int target_id,
                   const QString& target_name,
                   const QStringList& existing_backups);

private slots:
  /*!
   * \brief Called when the user edits the backup name field. Updates the Ok button.
   * \param text New text.
   */
  void on_name_field_textChanged(const QString& text);
  /*! \brief Emits \ref addBackupDialogAccepted with the data entered in the Ui. */
  void on_buttonBox_accepted();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddBackupDialog* ui;
  /*! \brief Id of the target for which to create a new backup. */
  int target_id_;
  /*! \brief Application to which the new backup is to be added. */
  int app_id_;
  /*! \brief Name of the target for which to create a new backup. */
  QString target_name_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

signals:
  /*!
   * \brief Signals completion of this dialog.
   * \param app_id Application to which the new backup is to be added.
   * \param target_id Id of the target for which to create a new backup.
   * \param name Name of the target for which to create a new backup.
   * \param target_name Name of the target for which to create a new backup.
   * \param source_backup Data for the new backup will be copied from this backup.
   */
  void addBackupDialogAccepted(int app_id,
                               int target_id,
                               QString name,
                               QString target_name,
                               int source_backup);
};
