/*!
 * \file editmodsourcesdialog.h
 * \brief Header for the EditModSourcesDialog class.
 */

#pragma once

#include <QDialog>
#include <QFileDialog>


namespace Ui
{
class EditModSourcesDialog;
}

/*!
 * \brief Used to edit the local and remote sources of a given mod.
 */
class EditModSourcesDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent widget of this dialog. Passed to the constructor of QDialog.
   */
  explicit EditModSourcesDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~EditModSourcesDialog();

  /*!
   * \brief Initializes the dialog with the given data.
   * \param app_id App to which the edited mod belongs.
   * \param mod_id Target mod id.
   * \param mod_name Target mod name.
   * \param local_source Path to a local archive or directory used for mod installation.
   * \param remote_source Remote URL from which the mod was downloaded.
   */
  void setupDialog(int app_id,
                   int mod_id,
                   const QString& mod_name,
                   const QString& local_source,
                   const QString& remote_source);
  /*!
   * \brief Emits dialogClosed.
   * \param event The close even sent upon closing the application.
   */
  void closeEvent(QCloseEvent* event) override;

private:
  /*! \brief Contains auto generated UI elements. */
  Ui::EditModSourcesDialog* ui;
  /*! \brief App to which the edited mod belongs. */
  int app_id_;
  /*! \brief Target mod id. */
  int mod_id_;
  /*! \brief Dialog used for selecting a local source. */
  std::unique_ptr<QFileDialog> file_dialog_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

private slots:
  /*!
   * \brief Updates the local source field with the new path.
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
   * \brief Signals successful dialog completion.
   * \param app_id App to which the edited mod belongs.
   * \param mod_id Target mod id.
   * \param local_source Path to a local archive or directory used for mod installation.
   * \param remote_source Remote URL from which the mod was downloaded.
   */
  void modSourcesEdited(int app_id, int mod_id, QString local_source, QString remote_source);
  /*! \brief Signals cancellation of editing. */
  void dialogClosed();
private slots:
  /*! \brief Completes the dialog by emitting modSourcesEdited. */
  void on_buttonBox_accepted();
  /*! \brief Signals cancellation of editing by emitting dialogClosed. */
  void on_buttonBox_rejected();
  /*! \brief Shows a file dialog for editing the local source path. */
  void on_path_picker_button_clicked();
};
