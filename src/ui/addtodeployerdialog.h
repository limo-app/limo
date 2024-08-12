/*!
 * \file addtodeployerdialog.h
 * \brief Header for the AddToDeployerDialog class.
 */

#pragma once

#include <QDialog>


namespace Ui
{
class AddToDeployerDialog;
}

/*!
 * \brief Dialog for choosing which deployers manage a given set of mods.
 */
class AddToDeployerDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddToDeployerDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddToDeployerDialog();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddToDeployerDialog* ui;
  /*! \brief Target mod ids. */
  std::vector<int> mod_ids_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

public:
  /*!
   * \brief Initializes the dialog.
   * \param deployer_names Names of all existing \ref Deployer "deployers".
   * \param mod_name Target mod's name. Only used if mod_ids.size() == 1.
   * \param mod_ids Target mods.
   * \param mod_deployers Ids of all \ref Deployer "deployers" managing the mod.
   */
  void setupDialog(const QStringList& deployer_names,
                   const QString& mod_name,
                   std::vector<int>& mod_ids,
                   const std::vector<int>& mod_deployers,
                   const std::vector<bool>& auto_deployers);

signals:
  /*!
   * \brief Signals completion of the dialog.
   * \param mod_id Target mods.
   * \param deployers Bool for every deployer, indicating if the mods should be managed
   * by that deployer.
   */
  void modDeployersUpdated(std::vector<int>& mod_id, std::vector<bool> deployers);

private slots:
  /*! \brief Closes the dialog and emits a signal for completion. */
  void on_buttonBox_accepted();
};
