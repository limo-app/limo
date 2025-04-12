/*!
 * \file addmoddialog.h
 * \brief Header for the AddModDialog class.
 */

#pragma once

#include "../core/importmodinfo.h"
#include "deployerlistmodel.h"
#include "modlistmodel.h"
#include "ui/fomoddialog.h"
#include <QButtonGroup>
#include <QCompleter>
#include <QDialog>
#include <QFrame>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <filesystem>


namespace Ui
{
class AddModDialog;
}

/*!
 * \brief Dialog for installing new mods.
 */
class AddModDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param mod_list_model Model used to store mod data.
   * \param deployer_list_model Model used to store deployer data.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddModDialog(ModListModel* mod_list_model,
                        DeployerListModel* deployer_list_model,
                        QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddModDialog();

  /*!
   * \brief Initializes this dialog with data needed for mod installation.
   * \param deployers Contains all available \ref Deployer "deployers".
   * \param cur_deployer The currently active Deployer.
   * \param deployer_paths Contains target paths for all non autonomous deployers.
   * \param autonomous_deployers Vector of bools indicating for each deployer
   * if that deployer is autonomous.
   * \param case_invariant_deployers Vector of bools indicating for each deployer
   * if that deployer is case invariant.
   * \param info Contains data relating to the current status of the mod import.
   * \return True if dialog creation was successful.
   */
  bool setupDialog(const QStringList& deployers,
                   int cur_deployer,
                   const QStringList& deployer_paths,
                   const std::vector<bool>& autonomous_deployers,
                   const std::vector<bool>& case_invariant_deployers,
                   const QString& app_version,
                   const ImportModInfo& info);
  /*!
   * \brief Closes the dialog and emits a signal indicating installation has been canceled.
   * \param event The close even sent upon closing the dialog.
   */
  void closeEvent(QCloseEvent* event) override;
  /*! \brief Closes the dialog and emits a signal indicating installation has been canceled. */
  void reject() override;

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddModDialog* ui;
  /*! \brief Holds radio button groups used to select installation options. */
  QList<QButtonGroup*> option_groups_;
  /*! \brief Used to color tree nodes which will not be removed. */
  const QColor COLOR_KEEP_{ 0x2ca02c };
  /*! \brief Used to color tree nodes which will be removed. */
  const QColor COLOR_REMOVE_{ 0xd62728 };
  /*! \brief Contains target paths for all deployers. */
  QStringList deployer_paths_;
  /*! \brief Prefix for fomod installer source path. */
  QString path_prefix_;
  /*! \brief Contains names of all available groups. */
  QStringList groups_;
  /*! \brief Completer used for group names. */
  std::unique_ptr<QCompleter> completer_;
  /*! \brief Dialog for fomod installation. */
  std::unique_ptr<FomodDialog> fomod_dialog_;
  /*! \brief Index in ui->group_combo_box representing the option of adding a mod to a group. */
  static constexpr int ADD_TO_GROUP_INDEX = 0;
  /*! \brief Index in ui->group_combo_box representing the option of replacing an existing mod. */
  static constexpr int REPLACE_MOD_INDEX = 1;
  /*! \brief App version used for fomod conditions. */
  QString app_version_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;
  /*! \brief Model containing all mod related data. */
  ModListModel* mod_list_model_;
  /*! \brief Model containing all deployer related data. */
  DeployerListModel* deployer_list_model_;
  /*! \brief For every deployer: A bool indicating whether that deployer is case invariant. */
  std::vector<bool> case_invariant_deployers_;
  /*! \brief Contains all data related to the current state of the mod installation. */
  ImportModInfo import_mod_info_;

  /*!
   * \brief Updates the enabled state of this dialog's OK button to only be enabled when
   * both a name and a version has been entered and an existing group or no group has been
   * selected.
   */
  void updateOkButton();
  /*!
   * \brief Adds the root path element of given path as a root node to the given tree.
   * Then adds all subsequent path components as children to the new node.
   * \param tree Target QTreeWidget.
   * \param cur_path Source path.
   */
  int addTreeNode(QTreeWidget* tree, const std::filesystem::path& cur_path);
  /*!
   * \brief Adds the root path element of given path as a root node to the given parent node.
   * Then adds all subsequent path components as children to the new node.
   * \param tree Target QTreeWidget.
   * \param cur_path Source path.
   * \return The depth of the given path.
   */
  int addTreeNode(QTreeWidgetItem* parent, const std::filesystem::path& cur_path);
  /*!
   * \brief Removes the root path component from the given path.
   * \param source Source path.
   * \return source without its root component.
   * \return The depth of the given path.
   */
  std::filesystem::path removeRoot(const std::filesystem::path& source);
  /*!
   * \brief Changes the color of the given node and its children, depending on
   * whether or not the nodes depth is less than the given root level.
   * \param node Node to be colored.
   * \param cur_depth Depth of current node.
   * \param root_level Target depth.
   */
  void colorTreeNodes(QTreeWidgetItem* node, int cur_depth, int root_level);
  /*!
   * \brief Shows a message box with a message constructed from given exception.
   * \param error Source of error.
   */
  void showError(const std::runtime_error& error);
  /*!
   * \brief Auto-detects the appropriate root level from the mod's content. Assumes that ui->content_tree
   * has been initialized.
   * \param deployer Deployer used to check for matching files.
   * \return The detected root level.
   */
  int detectRootLevel(int deployer) const;

private slots:
  /*! \brief Closes the dialog and emits a signal for completion. */
  void on_buttonBox_accepted();
  /*! \brief Enables or disables the group combo box depending on the check box state. */
  void on_group_check_stateChanged(int state);
  /*! \brief Closes the dialog and emits a signal indicating installation has been canceled. */
  void on_buttonBox_rejected();
  /*! \brief Only enable the OK button if a name has been entered. */
  void on_name_text_textChanged(const QString& text);
  /*! \brief Only enable the OK button if a version has been entered. */
  void on_version_text_textChanged(const QString& text);
  /*!
   * \brief Called when the value of the root level box has been changed by a user.
   * \param The new value.
   */
  void on_root_level_box_valueChanged(int value);
  /*!
   * \brief Enables/ disables ui elements based on chosen installer.
   * \param index New index.
   */
  void on_installer_box_currentIndexChanged(int index);
  /*!
   * \brief Updates the Ok buttons enabled state.
   * \param arg1 Ignored.
   */
  void on_group_field_textChanged(const QString& arg1);
  /*!
   * \brief Disables the deployer list if the new mod is to replace an existing mod.
   * \param index The new index.
   */
  void on_group_combo_box_currentIndexChanged(int index);
  /*!
   * \brief Called when the fomod dialog has been completed. Emits addModAccepted.
   * \param app_id Application for which the new mod is to be installed.
   * \param info Contains all data needed to install the mod.
   */
  void onFomodDialogComplete(int app_id, ImportModInfo info);
  /*! \brief Called when fomod dialog has been canceled. Emits addModAborted */
  void onFomodDialogAborted();

signals:
  /*!
   * \brief Signals dialog completion.
   * \param app_id Application for which the new mod is to be installed.
   * \param info Contains all data needed to install the mod.
   */
  void addModAccepted(int app_id, ImportModInfo info);
  /*!
   * \brief Signals mod installation has been aborted.
   * \param temp_dir Directory used for mod extraction.
   */
  void addModAborted(QString temp_dir);
};
