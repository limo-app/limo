/*!
 * \file addtooldialog.h
 * \brief Header for the AddToolDialog class.
 */

#pragma once

#include <QDialog>
#include "edittoolwidget.h"


namespace Ui
{
class AddToolDialog;
}

/*!
 * \brief Dialog for adding a new tool.
 */
class AddToolDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddToolDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddToolDialog();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddToolDialog* ui;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;
  /*!
   *  \brief If true: Dialog is used to edit an existing tool.
   *  Else: Dialog is used to add a new tool.
   */
  bool is_edit_mode_ = false;
  /*! \brief Id of the app to which the edited tool belongs. */
  int app_id_;
  /*! \brief If in edit mode: Id of the edited tool. */
  int tool_id_;

public:
  /*!
   * \brief Initializes the dialog for adding a new tool.
   * \param app_id Id of the app to which the tool is to be added.
   */
  void setAddMode(int app_id);
  /*!
   * \brief Initializes the dialog for editing an existing tool.
   * \param app_id Id of the app to which the edited tool belongs.
   * \param tool_id Id of the edited tool.
   * \param tool The existing tool.
   */
  void setEditMode(int app_id, int tool_id, Tool tool);

private slots:
  /*!
   * \brief Updates the Ok button when the new input is valid.
   * \param is_valid New input validity.
   */
  void toolWidgetInputValidityChanged(bool is_valid);
  /*! \brief Closes the dialog and emits a signal for completion. */
  void onButtonBoxAccepted();

signals:
  /*!
   * \brief Signals dialog completion in add mode.
   * \param app_id Id of the app to which the tool is to be added.
   * \param tool The new tool.
   */
  void toolAdded(int app_id, Tool tool);
  /*!
   * \brief Signals dialog completion in edit mode.
   * \param app_id Id of the app to which the edited tool belongs.
   * \param tool_id Id of the edited tool.
   * \param tool The new tool.
   */
  void toolEdited(int app_id, int tool_id, Tool tool);
};
