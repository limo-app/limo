/*!
 * \file addprofiledialog.h
 * \brief Header for the AddProfileDialog class.
 */

#pragma once

#include "../core/editprofileinfo.h"
#include <QDialog>


namespace Ui
{
class AddProfileDialog;
}

/*!
 * \brief Dialog for creating and editing profiles.
 */
class AddProfileDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddProfileDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddProfileDialog();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddProfileDialog* ui;
  /*! \brief If true: Dialog is used to edit, else: Dialog is used to create. */
  bool edit_mode_ = false;
  /*! \brief Target ModdedApplication. */
  int app_id_;
  /*! \brief Target profile. */
  int profile_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

public:
  /*!
   * \brief Initializes the dialog to allow creating a new profile.
   * \param app_id Target ModdedApplication.
   * \param profiles Names of existing profiles.
   */
  void setAddMode(int app_id, const QStringList& profiles);
  /*!
   * \brief Initializes the dialog to allow editing an existing profile.
   * \param app_id Target ModdedApplication.
   * \param profile Profile to be edited.
   * \param name Current name of the edited profile.
   * \param app_version Current app version for this profile.
   */
  void setEditMode(int app_id, int profile, const QString& name, const QString& app_version);

private slots:
  /*! \brief Enables/ disables clone combo box to reflect the state of the clone check box. */
  void on_clone_check_box_stateChanged(int state);
  /*! \brief Only enable the OK button if a name has been entered. */
  void on_name_field_textChanged(const QString& text);
  /*! \brief Closes this dialog and emits a signal for completion. */
  void on_buttonBox_accepted();

signals:
  /*!
   * \brief Signals completion of this dialog in add mode.
   * \param app_id Application for which a new profile is to be added.
   * \param info Contains data for the new profile.
   */
  void profileAdded(int app_id, EditProfileInfo info);
  /*!
   * \brief Signals completion of this dialog in edit mode.
   * \param app_id Target application.
   * \param profile Profile to be edited.
   * \param info Contains the new data for the edited profile.
   */
  void profileEdited(int app_id, int profile, EditProfileInfo info);
};
