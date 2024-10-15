/*!
 * \file exportappconfigdialog.h
 * \brief Header for the ExportAppConfigDialog class.
 */

#pragma once

#include <QDialog>


namespace Ui
{
class ExportAppConfigDialog;
}

/*!
 * \brief Dialog used for choosing which deployers and auto tags to export for a given app.
 */
class ExportAppConfigDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit ExportAppConfigDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~ExportAppConfigDialog();

  /*!
   * \brief Initializes this dialog with data regarding deployers and auto tags.
   * \param app_id Target app id.
   * \param app_name Target app name.
   * \param deployers Names of deployers used by the target app. Must be on order.
   * \param auto_tags Auto tags used by the target app.
   */
  void init(int app_id,
            const QString& app_name,
            const QStringList& deployers,
            const QStringList& auto_tags);

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::ExportAppConfigDialog* ui;
  /*! \brief Target app id. */
  int app_id_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;
  /*!
   * \brief Closes the dialog and emits a signal indicating app export has been canceled.
   * \param event The close event sent upon closing the dialog.
   */
  void closeEvent(QCloseEvent* event) override;
  /*! \brief Closes the dialog and emits a signal indicating the dialog has been closed. */
  void reject() override;

signals:
  /*!
   * \brief Signals that the dialog has been accepted. Sends selected deployers and auto tags.
   * \param app_id Target app id.
   * \param deployers Selected deployers ids.
   * \param auto_tags Selected auto tag names.
   */
  void appConfigExported(int app_id, std::vector<int> deployers, QStringList auto_tags);
  /*! \brief Signals cancellation of export. */
  void dialogClosed();
private slots:
  /*! \brief Closes the dialog and emits \ref appConfigExported. */
  void on_buttonBox_accepted();
  /*! \brief Closes the dialog and emits \ref dialogClosed. */
  void on_buttonBox_rejected();
};
