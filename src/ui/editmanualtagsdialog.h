/*!
 * \file editmanualtagsdialog.h
 * \brief Header for the EditManualTagsDialog class.
 */

#pragma once

#include "../core/editmanualtagaction.h"
#include <QDialog>
#include <vector>


namespace Ui
{
class EditManualTagsDialog;
}

/*!
 * \brief Dialog used to add, remove or rename manual tags.
 */
class EditManualTagsDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit EditManualTagsDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~EditManualTagsDialog();

  /*!
   * \brief Initializes this dialog with the given data.
   * \param app_id App for which the tags are to be edited.
   * \param tag_names Names of all manual tags belonging to the given app. Names must be unique.
   * \param num_mods_per_tag Number of mods for every given tag.
   */
  void setupDialog(int app_id,
                   const QStringList& tag_names,
                   const std::vector<int> num_mods_per_tag);
  /*!
   * \brief Emits dialogClosed.
   * \param event The close event sent upon closing the dialog.
   */
  void closeEvent(QCloseEvent* event) override;

private:
  /*! \brief Action column in the tag table. */
  static constexpr int ACTION_COL = 0;
  /*! \brief Tag name column in the tag table. */
  static constexpr int NAME_COL = 1;
  /*! \brief Number of mods column in the tag table. */
  static constexpr int NUM_MODS_COL = 2;

  /*! \brief Contains auto-generated UI elements. */
  Ui::EditManualTagsDialog* ui;
  /*! \brief App for which the tags are to be edited. */
  int app_id_;
  /*! \brief Contains names of all tags. Names are unique. */
  QStringList tag_names_;
  /*! \brief For every tag: The number of mods to which the tag has been added. */
  std::vector<int> num_mods_per_tag_;
  /*! \brief Contains all actions performed in this dialog. Emitted after dialog completes. */
  std::vector<EditManualTagAction> actions_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

  /*! \brief Updates the tag table with data stored in tag_names_ and num_mods_per_tag_. */
  void updateTable();

private slots:
  /*!
   * \brief Removed the tag in the given row of the tag table.
   * \param row Target row.
   * \param col Contains the buttons column. Not used.
   */
  void onTagRemoved(int row, int col);
  /*! \brief Adds a new tag. */
  void onTagAdded();
  /*!
   * \brief Renames the tag in the given row.
   * \param row Target row.
   * \param col Column of the tag name. Not used.
   */
  void onTableCellEdited(int row, int col);
  /*! \brief Emits manualTagsEdited with all actions performed in this dialog. */
  void on_buttonBox_accepted();
  /*! \brief Emits a signal that this dialog has been closed without changes. */
  void on_buttonBox_rejected();

signals:
  /*!
   * \brief Signals dialog completion.
   * \param app_id App for which the tags are to be edited.
   * \param actions All actions performed in this dialog in the order in which they have been
   * performed by the user.
   */
  void manualTagsEdited(int app_id, std::vector<EditManualTagAction> actions);
  /*! \brief Signals tag editing has been cancled without action. */
  void dialogClosed();
};
