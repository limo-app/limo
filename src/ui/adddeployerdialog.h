/*!
 * \file adddeployerdialog.h
 * \brief Header for the AddDeployerDialog class.
 */

#pragma once

#include "../core/editdeployerinfo.h"
#include <QDialog>


namespace Ui
{
class AddDeployerDialog;
}

/*!
 * \brief Dialog for creating and editing \ref Deployer "deployers".
 */
class AddDeployerDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddDeployerDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddDeployerDialog();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddDeployerDialog* ui;
  /*! \brief If true: Dialog is used to edit, else: Dialog is used to create. */
  bool edit_mode_ = false;
  /*! \brief Current name of the edited Deployer. */
  QString name_;
  /*! \brief Current target directory of the edited Deployer. */
  QString path_;
  /*! \brief Current type of the edited Deployer. */
  QString type_;
  /*! \brief  Id of the ModdedApplication owning the edited Deployer. */
  int app_id_;
  /*! \brief Id of the edited Deployer. */
  int deployer_id_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

  /*!
   * \brief Set the enabled state of this dialog's OK button.
   * \param state
   */
  void enableOkButton(bool state);
  /*! \brief Checks whether the currently entered path exists. */
  bool pathIsValid();
  /*! \brief Adds all available Deployer types to the type combo box. */
  void setupTypeBox();
  /*! \brief Updates the state of this dialog's OK button to only be enabled when all inputs are
   * valid. */
  void updateOkButton();

public:
  /*!
   *  \brief Initializes this dialog to allow creating a new Deployer.
   *  \param app_id Id of the ModdedApplication owning the edited Deployer.
   */
  void setAddMode(int app_id);
  /*!
   * \brief setEditMode Initializes this dialog to allow editing an existing Deployer.
   * \param type Current type of the edited Deployer.
   * \param name Current name of the edited Deployer.
   * \param path Current target directory of the edited Deployer.
   * \param app_id Id of the ModdedApplication owning the edited Deployer.
   * \param deployer_id Id of the edited Deployer.
   */
  void setEditMode(QString type,
                   QString name,
                   QString path,
                   bool use_copy_deployment,
                   int app_id,
                   int deployer_id);
  /*! \brief Enables/ Disables the ui elements responsible for setting a source directory. */
  void updateSourceFields();

private slots:
  /*! \brief Shows a file dialog for the target directory path. */
  void on_file_picker_button_clicked();
  /*! \brief Only enable the OK button if a name has been entered. */
  void on_name_field_textChanged(const QString& text);
  /*! \brief Only enable the OK button if a valid target directory path has been entered. */
  void on_path_field_textChanged(const QString& text);
  /*! \brief Closes the dialog and emits a signal for completion. */
  void on_buttonBox_accepted();
  /*! \brief Updates the target path with given path. */
  void onFileDialogAccepted(const QString& path);
  /*! \brief Updates the warning label. */
  void on_copy_box_stateChanged(int arg1);
  /*! \brief Updates the source path widgets enabled status. */
  void on_type_box_currentIndexChanged(int index);
  /*! \brief Shows a file dialog for the source directory path. */
  void on_source_picker_button_clicked();
  /*! \brief Updates the source path with given path. */
  void onSourceDialogAccepted(const QString& path);
  /*! \brief Only enable the OK button if a valid source directory path has been entered. */
  void on_source_path_field_textChanged(const QString& path);

signals:
  /*!
   * \brief Signals completion of the dialog in add mode.
   * \param info Contains all data entered in this dialog.
   * \param app_id Id of the ModdedApplication owning the edited Deployer.
   */
  void deployerAdded(EditDeployerInfo info, int app_id);
  /*!
   * \brief Signals completion of the dialog in edit mode.
   * \param info Contains all data entered in this dialog.
   * \param app_id Id of the ModdedApplication owning the edited Deployer.
   * \param deployer_id Id of the edited Deployer.
   */
  void deployerEdited(EditDeployerInfo info, int app_id, int deployer_id);
};
