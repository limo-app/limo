/*!
 * \file addtogroupdialog.h
 * \brief Header for the AddToGroupDialog class.
 */

#pragma once

#include <QCompleter>
#include <QDialog>
#include <vector>


namespace Ui
{
class AddToGroupDialog;
}

/*!
 * \brief Dialog for adding a mod to a group.
 */
class AddToGroupDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddToGroupDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddToGroupDialog();

  /*!
   * \brief Initializes the dialog.
   * \param groups Names of all installed mods except the target mod.
   * \param mod_ids Ids for all installed mods except the target mod.
   * \param mod_name Name of the mod which is to be added to a group.
   * \param mod_id Id of the mod which is to be added to a group.
   */
  void setupDialog(const QStringList& groups,
                   const std::vector<int>& mod_ids,
                   const QString& mod_name,
                   int mod_id);

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddToGroupDialog* ui;
  /*! \brief Id of the mod which is to be added to a group. */
  int mod_id_;
  /*! \brief Ids of all installed mods except the target mod. */
  std::vector<int> mod_ids_;
  /*! \brief Completer used for group names. */
  std::unique_ptr<QCompleter> completer_;
  /*! \brief Contains names of all available groups. */
  QStringList groups_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

signals:
  /*!
   * \brief Signals completion of the dialog.
   * \param mod_id Id Id of the mod which is to be added to a group.
   * \param target_id Id of the mod to whose group the mod will be added.
   */
  void modAddedToGroup(int mod_id, int target_id);
private slots:
  /*! \brief Closes the dialog and emits a signal for completion. */
  void on_buttonBox_accepted();
  /*!
   * \brief Ensures Ok button is only available when a valid group has been selected.
   * \param text The new input.
   */
  void on_group_field_textChanged(const QString& text);
};
