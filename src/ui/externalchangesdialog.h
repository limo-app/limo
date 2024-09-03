/*!
 * \file externalchangesdialog.h
 * \brief Header for the ExternalChangesDialog class.
 */

#pragma once

#include "core/externalchangesinfo.h"
#include "core/filechangechoices.h"
#include <QDialog>


namespace Ui
{
class ExternalChangesDialog;
}

/*!
 * \brief Dialog for selecting which external changes should be kept.
 */
class ExternalChangesDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit ExternalChangesDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~ExternalChangesDialog();

  /*!
   * \brief Initializes the dialog.
   * \param app_id Id of the app containing the modified files.
   * \param info Contains data regarding which files have been modified and to which mods those files belong.
   */
  void setup(int app_id, const ExternalChangesInfo& info);

private slots:
  /*! \brief Signals sucessful dialog completion. Emits \ref externalChangesDialogCompleted. */
  void on_buttonBox_accepted();
  /*! \brief Signals dialog has been aborted. Emits \ref externalChangesDialogAborted. */
  void on_buttonBox_rejected();
  /*! \brief Checks all entries in the file list. */
  void on_actionKeep_All_triggered();
  /*! \brief Unchecks all entries in the file list. */
  void on_actionKeep_None_triggered();
  /*! \brief Toggles all selected entries in the file list. */
  void on_actionToggle_Selected_triggered();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::ExternalChangesDialog* ui;
  /*! \brief Id of the app containing the modified files. */
  int app_id_ = -1;
  /*! \brief Contains data regarding which files have been modified and to which mods those files belong. */
  ExternalChangesInfo changes_info_;

signals:
  /*!
   * \brief Signals sucessful dialog completion.
   * \param app_id Id of the app containing the modified files.
   * \param deployer Id of the deployer managing the modified files.
   * \param changes_to_keep Contains data regarding which modifications to keep and which to reject.
   */
  void externalChangesDialogCompleted(
    int app_id,
    int deployer,
    const FileChangeChoices& changes_to_keep);
  /*! \brief Signals dialog has been aborted. */
  void externalChangesDialogAborted();
};
