/*!
 * \file overwritedialog.h
 * \brief Header for the OverwriteBackupDialog class.
 */

#pragma once

#include <QCompleter>
#include <QDialog>


namespace Ui
{
class OverwriteBackupDialog;
}

/*!
 * \brief Dialog for overwriting backups.
 */
class OverwriteBackupDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit OverwriteBackupDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~OverwriteBackupDialog();
  /*!
   * \brief Initializes the dialog.
   * \param backup_names Contains names for all backups.
   * \param target_id Id of the backup target for which to overwrite a backup.
   * \param dest_backup Backup to be overwritten.
   */
  void setupDialog(const QStringList& backup_names, int target_id, int dest_backup);

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::OverwriteBackupDialog* ui;
  /*! \brief Id of the backup target for which to overwrite a backup. */
  int backup_target_;
  /*! \brief Backup to be overwritten. */
  int dest_backup_;
  /*! \brief Contains names for all backups. */
  QStringList backup_names_;
  /*! \brief Completer used for backup names. */
  std::unique_ptr<QCompleter> completer_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

private slots:
  /*!
   * \brief Ensures Ok button is only available when a valid backup has been selected.
   * \param text The new input.
   */
  void on_backup_field_textChanged(const QString& text);
  /*! \brief Closes the dialog and emits \ref backupOverwritten. */
  void on_buttonBox_accepted();

signals:
  /*!
   * \brief Signals completion of the dialog.
   * \param target_id Id of the backup target for which to overwrite a backup.
   * \param source_backup Backup from which to copy files.
   * \param dest_backup Backup to be overwritten.
   */
  void backupOverwritten(int target_id, int source_backup, int dest_backup);
};
