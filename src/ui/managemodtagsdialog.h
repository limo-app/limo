/*!
 * \file managemodtagsdialog.h
 * \brief Header for the ManageModTagsDialog class.
 */

#pragma once

#include <QDialog>

namespace Ui
{
class ManageModTagsDialog;
}

/*!
 * \brief Dialog for choosing which deployers manage a given set of mods.
 */
class ManageModTagsDialog : public QDialog
{
  Q_OBJECT

public:
  /*! \brief Index for add mode in the mode combo box. */
  static constexpr int add_mode = 0;
  /*! \brief Index for remove mode in the mode combo box. */
  static constexpr int remove_mode = 1;
  /*! \brief Index for overwrite mode in the mode combo box. */
  static constexpr int overwrite_mode = 2;

  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit ManageModTagsDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~ManageModTagsDialog();

  /*!
   * \brief Initializes the dialog.
   * \param app_id Target app.
   * \param tags All manual tags available in the current ModdedApplication.
   * \param mod_tags All tags assigned to the first selected mod.
   * \param mod_name Name of the first selected mod.
   * \param mod_ids Ids for all selected mods.
   */
  void setupDialog(int app_id,
                   const QStringList& tags,
                   const QStringList& mod_tags,
                   const QString& mod_name,
                   const std::vector<int>& mod_ids);
  /*!
   * \brief Emits dialogClosed.
   * \param event The close event sent upon closing the dialog.
   */
  void closeEvent(QCloseEvent* event) override;
  /*! \brief Closes the dialog and emits a signal indicating the dialog has been closed. */
  void reject() override;

private:
  /*! \brief ModdedApplication to which the tags belong. */
  int app_id_;
  /*! \brief Contains auto-generated UI elements. */
  Ui::ManageModTagsDialog* ui;
  /*! \brief Target mod ids. */
  std::vector<int> mod_ids_;
  /*! \brief Name of the first selected mod. */
  QString mod_name_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

  /*! \brief Updates the text above the tag list according to current settings. */
  void updateHintText();

signals:
  /*!
   * \brief Signals completion of the dialog.
   * \param app_id Target ModdedApplication.
   * \param tags All tags to be assigned for all given mods.
   * \param mod_ids Ids of the mods for which tags are to be updated.
   * \param mode Indicates whether tags should be added, removed or overwritten.
   */
  void modTagsUpdated(int app_id, QStringList tags, std::vector<int> mod_ids, int mode);
  /*! \brief Emitted when the dialog has been closed by pressing the cancel button. */
  void dialogClosed();

private slots:
  /*! \brief Closes the dialog and emits a signal for completion. */
  void on_buttonBox_accepted();
  /*! \brief Closes the dialog and emits a signal for cancelation. */
  void on_buttonBox_rejected();
  /*!
   * \brief Calls updateHintText.
   * \param index New index.
   */
  void on_mode_box_currentIndexChanged(int index);
};
