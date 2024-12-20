/*!
 * \file addmoddialog.h
 * \brief Header for the AddModDialog class.
 */

#pragma once

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
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddModDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddModDialog();

  /*!
   * \brief Initializes this dialog with data needed for mod installation.
   * \param name Default mod name.
   * \param deployers Contains all available \ref Deployer "deployers".
   * \param cur_deployer The currently active Deployer.
   * \param groups Contains all mod names which act as targets for groups.
   * \param mod_ids Ids of all currently installed mods.
   * \param path Source path for the new mod.
   * \param deployer_paths Contains target paths for all non autonomous deployers.
   * \param app_id Id of currently active application.
   * \param autonomous_deployers Vector of bools indicating for each deployer
   * if that deployer is autonomous.
   * \param local_source Source archive for the mod.
   * \param remote_source URL from where the mod was downloaded.
   * \param mod_id If =! -1: Id of the mod to the group of which the new mod should be added by default.
   * \param mod_names Contains the name of all currently installed mods.
   * \param mod_versions Contains the versions of all currently installed mods.
   * \param version_overwrite If not empty: Use this to overwrite the default version.
   * \return True if dialog creation was successful.
   */
  bool setupDialog(const QString& name,
                   const QStringList& deployers,
                   int cur_deployer,
                   const QStringList& groups,
                   const std::vector<int>& mod_ids,
                   const QString& path,
                   const QStringList& deployer_paths,
                   int app_id,
                   const std::vector<bool>& autonomous_deployers,
                   const QString& app_version,
                   const QString& local_source,
                   const QString& remote_source,
                   int mod_id,
                   const QStringList& mod_names,
                   const QStringList& mod_versions,
                   const QString& version_overwrite);
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
  /*! \brief Contains mod ids corresponding to entries in the field. */
  std::vector<int> mod_ids_;
  /*! \brief Source path for the new mod data. */
  QString mod_path_;
  /*! \brief Stores the id of the currently active \ref ModdedApplication "application". */
  int app_id_;
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
  /*! \brief Path to the source archive for the mod. */
  QString local_source_;
  /*! \brief URL from where the mod was downloaded. */
  QString remote_source_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

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
  void onFomodDialogComplete(int app_id, AddModInfo info);
  /*! \brief Called when fomod dialog has been canceled. Emits addModAborted */
  void onFomodDialogAborted();

signals:
  /*!
   * \brief Signals dialog completion.
   * \param app_id Application for which the new mod is to be installed.
   * \param info Contains all data needed to install the mod.
   */
  void addModAccepted(int app_id, AddModInfo info);
  /*!
   * \brief Signals mod installation has been aborted.
   * \param temp_dir Directory used for mod extraction.
   */
  void addModAborted(QString temp_dir);
};
