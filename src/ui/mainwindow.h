/*!
 * \file mainwindow.h
 * \brief Header for the MainWindow class.
 */

#pragma once

#include "addappdialog.h"
#include "addbackupdialog.h"
#include "addbackuptargetdialog.h"
#include "adddeployerdialog.h"
#include "addmoddialog.h"
#include "addprofiledialog.h"
#include "addtodeployerdialog.h"
#include "addtogroupdialog.h"
#include "addtooldialog.h"
#include "applicationmanager.h"
#include "backuplistmodel.h"
#include "backupnamedelegate.h"
#include "conflictsmodel.h"
#include "core/importmodinfo.h"
#include "deployerlistmodel.h"
#include "deployerlistproxymodel.h"
#include "editmodsourcesdialog.h"
#include "managemodtagsdialog.h"
#include "modlistmodel.h"
#include "modlistproxymodel.h"
#include "modnamedelegate.h"
#include "nexusmoddialog.h"
#include "overwritebackupdialog.h"
#include "settingsdialog.h"
#include "tablecelldelegate.h"
#include "ui/changelogdialog.h"
#include "ui/editautotagsdialog.h"
#include "ui/editmanualtagsdialog.h"
#include "ui/exportappconfigdialog.h"
#include "ui/externalchangesdialog.h"
#include "ui/ipcserver.h"
#include "ui/listaction.h"
#include "ui/rootlevelcondition.h"
#include "ui/tagcheckbox.h"
#include "versionboxdelegate.h"
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QTableWidget>
#include <QThread>
#include <QtCore>


QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

/*!
 * \brief Represents the main window of the application and contains slots for all GUI elements
 * and its widgets.
 */
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  /*! \brief Describes target widgets for logging. */
  enum LogFrame
  {
    /*! \brief Main logging widget. */
    LOG_MAIN = 0,
    /*! \brief Logging widget for tools. */
    LOG_TOOLS = 1
  };

  /*!
   * \brief Constructor. Initializes the UI elements and restores their state from a file.
   * Creates an ApplicationManager object and moves it to a worker thread.
   * Communication with the thread is done using Qt's signal/ slot mechanism.
   * \param parent The parent widget of this object.
   */
  MainWindow(QWidget* parent = nullptr);
  /*!
   * \brief Destructor. Stops the worker thread and performs cleanup.
   */
  ~MainWindow();
  /*!
   * \brief Store the GUI state in a QSettings file before closing the window.
   * \param event The close event sent upon closing the application.
   */
  void closeEvent(QCloseEvent* event) override;
  /*!
   * \brief Checks if the given argument is a NexusMods download link. If True: Downloads the mod.
   * \param argument Potential download link.
   */
  void setCmdArgument(std::string argument);
  /*!
   * \brief Enables/ disables printing of debug log messages.
   * \param enabled If true: Print debug messages.
   */
  void setDebugMode(bool enabled);
  /*!
   *  \brief Initializes the changelogdialog. Show the dialog if
   *  current version > version on last start
   */
  void initChangelog();

private:
  /*! \brief Contains auto-generated ui elements. */
  Ui::MainWindow* ui;
  /*! \brief Index of the app tab. */
  static constexpr int app_tab_idx = 0;
  /*! \brief Index of the app tab. */
  static constexpr int mods_tab_idx = 1;
  /*! \brief Index of the app tab. */
  static constexpr int deployer_tab_idx = 2;
  /*! \brief Index of the app tab. */
  static constexpr int backup_tab_idx = 3;
  /*! \brief Display string for hard link deployment. */
  static inline const QString deploy_mode_hard_link = "Hard Link";
  /*! \brief Display string for sym link deployment. */
  static inline const QString deploy_mode_sym_link = "Sym Link";
  /*! \brief Display string for copy deployment. */
  static inline const QString deploy_mode_copy = "Copy";
  /*! \brief JSON key for the root level conditions in the per steam app config file. */
  static inline constexpr char JSON_ROOT_LEVEL_KEY[] = "root_level_conditions";
  /*! \brief True if the button used to reorder load orders is being pressed. */
  bool move_button_pressed_ = false;
  /*! \brief Stores the row containing the currently held down move button for load orders. */
  int move_button_row_ = 0;
  /*! \brief Handles installation and deployment of all mods. Runs in a separate thread. */
  ApplicationManager* app_manager_;
  /*! \brief Thread containing the ApplicationManager. */
  QThread* worker_thread_;
  /*! \brief If true: changes to ui->mod_list will not trigger table updates. */
  bool ignore_table_changes_ = false;
  /*! \brief Indicates whether ui->deployer_list has been initialized. */
  bool is_initialized_ = false;
  /*! \brief Matched against mod names to filter the mod- and deployer lists. */
  QString search_term_ = "";
  /*! \brief Context menu for ui->mod_list. */
  QMenu* mod_list_menu_;
  /*! \brief Context menu for ui->deployer_list. */
  QMenu* deployer_list_menu_;
  /*! \brief Context menu for ui->backup_list. */
  QMenu* backup_list_menu_;
  /*! \brief Action used to run the current \ref ModdedApplication "application". */
  QAction* run_app_action_;
  /*! \brief Action used to add a new \ref ModdedApplication "application". */
  QAction* add_app_action_;
  /*! \brief Action used to remove the current \ref ModdedApplication "application". */
  QAction* remove_app_action_;
  /*! \brief Action used to edit the current \ref ModdedApplication "application". */
  QAction* edit_app_action_;
  /*! \brief Action used to add a new \ref Deployer "deployer". */
  QAction* add_deployer_action_;
  /*! \brief Action used to remove the current \ref Deployer "deployer". */
  QAction* remove_deployer_action_;
  /*! \brief Action used to edit the current \ref Deployer "deployer". */
  QAction* edit_deployer_action_;
  /*! \brief Action used to add a new profile. */
  QAction* add_profile_action_;
  /*! \brief Action used to remove a profile. */
  QAction* remove_profile_action_;
  /*! \brief Action used to edit the current profile. */
  QAction* edit_profile_action_;
  /*! \brief If true: changes to ui->info_tool_list will not trigger table updates. */
  bool ignore_tool_changes_ = false;
  /*! \brief If true: Show confirmation box before removing a Deployer. */
  bool ask_remove_from_deployer_ = true;
  /*! \brief If true: Show confirmation box before removing a mod. */
  bool ask_remove_mod_ = true;
  /*! \brief If true: Show confirmation box before removing a profile. */
  bool ask_remove_profile_ = true;
  /*! \brief If true: Show confirmation box before removing a backup target. */
  bool ask_remove_backup_target_ = true;
  /*! \brief If true: Show confirmation box before removing a backup. */
  bool ask_remove_backup_ = true;
  /*! \brief If true: Show confirmation box before removing a tool. */
  bool ask_remove_tool_ = true;
  /*! \brief Reusable dialog for adding new \ref ModdedApplication "applications". */
  std::unique_ptr<AddAppDialog> add_app_dialog_;
  /*! \brief Reusable dialog for adding new \ref Deployer "deployers". */
  std::unique_ptr<AddDeployerDialog> add_deployer_dialog_;
  /*! \brief Reusable dialog for adding new mods. */
  std::unique_ptr<AddModDialog> add_mod_dialog_;
  /*! \brief Reusable dialog for adding new profiles. */
  std::unique_ptr<AddProfileDialog> add_profile_dialog_;
  /*! \brief Reusable dialog for adding a mod to a Deployer. */
  std::unique_ptr<AddToDeployerDialog> add_to_deployer_dialog_;
  /*! \brief Reusable dialog for adding new tools. */
  std::unique_ptr<AddToolDialog> add_tool_dialog_;
  /*! \brief Used to show info or error messages. */
  std::unique_ptr<QMessageBox> message_box_;
  /*! \brief Reusable dialog for adding a mod to a group. */
  std::unique_ptr<AddToGroupDialog> add_to_group_dialog_;
  /*! \brief Reusable settings dialog. */
  std::unique_ptr<SettingsDialog> settings_dialog_;
  /*! \brief Reusable dialog for adding new backup targets. */
  std::unique_ptr<AddBackupTargetDialog> add_backup_target_dialog_;
  /*! \brief Reusable dialog for adding new backups to existing targets. */
  std::unique_ptr<AddBackupDialog> add_backup_dialog_;
  /*! \brief Reusable dialog for overwriting backups. */
  std::unique_ptr<OverwriteBackupDialog> overwrite_backup_dialog_;
  /*! \brief Reusable dialog for editing manual tags. */
  std::unique_ptr<EditManualTagsDialog> edit_manual_tags_dialog_;
  /*! \brief Reusable dialog for managing the manual tags assigned to a set of mods. */
  std::unique_ptr<ManageModTagsDialog> manage_mod_tags_dialog_;
  /*! \brief Reusable dialog for editing auto tags. */
  std::unique_ptr<EditAutoTagsDialog> edit_auto_tags_dialog_;
  /*! \brief Reusable dialog for editing local and remote mod source paths. */
  std::unique_ptr<EditModSourcesDialog> edit_mod_sources_dialog_;
  /*! \brief Reusable dialog for displaying NexusMods data for a mod. */
  std::unique_ptr<NexusModDialog> nexus_mod_dialog_;
  /*! \brief Reusable dialog for displaying external changes to files. */
  std::unique_ptr<ExternalChangesDialog> external_changes_dialog_;
  /*! \brief Reusable dialog for exporting the configuration of the current app. */
  std::unique_ptr<ExportAppConfigDialog> export_app_config_dialog_;
  /*! \brief Reusable dialog for showing changelogs. */
  std::unique_ptr<ChangelogDialog> changelog_dialog_;
  /*! \brief Stores the index in ui->mod_list of a mod before being added to a group. */
  int last_mod_list_index_ = -1;
  /*! \brief Contains all queued mods to be downloaded or extracted. */
  std::priority_queue<ImportModInfo> mod_import_queue_;
  /*! \brief Last position of the scroll bar for ui->deployer_list */
  int deployer_list_slider_pos_ = 0;
  /*! \brief Last position of the scroll bar for ui->mod_list */
  int mod_list_slider_pos_ = 0;
  /*! \brief Temporary directory used for file extractions. */
  const QString temp_dir_ = "lmm_tmp_extract_.dir";
  /*! \brief Model used by the deployer list. */
  DeployerListModel* deployer_model_;
  /*! \brief Model used by the mod list. */
  ModListModel* mod_list_model_;
  /*! \brief Used to edit mod versions in the mod list. */
  VersionBoxDelegate* version_deledate_;
  /*! \brief Used to edit mod names in the mod list. */
  ModNameDelegate* mod_name_delegate_;
  /*! \brief Proxy model for the mod list. */
  ModListProxyModel* mod_list_proxy_;
  /*! \brief Proxy model for the deployer list. */
  DeployerListProxyModel* deployer_list_proxy_;
  /*! \brief Model used by the backup list. */
  BackupListModel* backup_list_model_;
  /*! \brief Used to edit the active backup in the backup list. */
  VersionBoxDelegate* backup_delegate_;
  /*! \brief Used to edit the name of backup targets. */
  BackupNameDelegate* backup_target_name_delegate_;
  /*! \brief Delegate used to draw cells in ui->mod_list. */
  TableCellDelegate* mod_list_cell_delegate_;
  /*! \brief Delegate used to draw cells in ui->deployer_list. */
  TableCellDelegate* deployer_list_cell_delegate_;
  /*! \brief Delegate used to draw cells in ui->backup_list. */
  TableCellDelegate* backup_list_cell_delegate_;
  /*! \brief Model used to hold data for file conflicts. */
  ConflictsModel* conflicts_model_;
  /*! \brief Holds \ref conflicts_list_. */
  QWidget* conflicts_window_;
  /*! \brief Used to display file conflicts. */
  QTableView* conflicts_list_;
  /*! \brief Progress bar shown in the status bar. */
  QProgressBar* progress_bar_;
  /*!
   *  \brief Maps the names of all manual tags for the current app to the number of mods with that
   * tag.
   */
  std::map<std::string, int> num_mods_per_manual_tag_;
  /*! \brief Pointers to all checkboxes used to filter the mod list by manual tags. */
  std::vector<TagCheckBox*> manual_tag_cbs_;
  /*! \brief Pointers to all checkboxes used to filter the deployer list by tags. */
  std::vector<TagCheckBox*> depl_tag_cbs_;
  /*!
   *  \brief Maps the names of all manual tags for the current app to the number of mods with that
   * tag.
   */
  std::map<std::string, int> num_mods_per_auto_tag_;
  /*! \brief Pointers to all checkboxes used to filter the mod list by auto tags. */
  std::vector<TagCheckBox*> auto_tag_cbs_;
  /*!
   * \brief Maps all auto tag names of the current app to a pair of the expression
   * used and a vector of Tagconditions.
   */
  std::map<std::string, std::pair<std::string, std::vector<TagCondition>>> auto_tags_;
  /*!
   *  \brief If true: Deploy button starts deployment for all deployers,
   *  else: Only for the currently active one.
   */
  bool deploy_for_all_ = true;
  /*! \brief If true: Show the log window when an error is logged. */
  bool show_log_on_error_ = false;
  /*! \brief If true: Show the log window when a warning is logged. */
  bool show_log_on_warning_ = false;
  /*! \brief QLocalServer wrapper used for communication with other instances of Limo. */
  std::unique_ptr<IpcServer> ipc_server_;
  /*! \brief Timestamp representing the last time the progress bar has been updated. */
  std::chrono::time_point<std::chrono::high_resolution_clock> last_progress_update_time_;
  /*! \brief The last progress state. */
  float last_progress_;
  /*! \brief Tracks whether a progress signals has been received since the last busy action started. */
  bool received_progress_ = false;
  /*! \brief If true: Print debug messages. */
  bool debug_mode_ = false;
  /*! \brief Indicates if Limo is running as a flatpak. */
  bool is_a_flatpak_ = false;
  /*! \brief For every current deployer: The source path it uses. */
  std::vector<QString> deployer_source_paths_;
  /*! \brief For every current deployer: The target path it uses. */
  std::vector<QString> deployer_target_paths_;
  /*! \brief Contains all tools managed by the current app. */
  std::vector<Tool> tools_;
  /*!
   *  \brief Contains additional mod actions to be added to
   *  the deployer list context menu for the current deployer.
   */
  std::vector<ListAction*> deployer_mod_actions_;
  /*! \brief APP_VERSION the last time the app was run. */
  QString previous_app_version_;
  /*! \brief Contains data about the currently active application. */
  AppInfo app_info_;
  /*! \brief Used to detect root levels during mod installation for the current app. */
  std::vector<RootLevelCondition> root_level_conditions_;

  /*! \brief Creates signal/ slot connections between this and the ApplicationManager. */
  void setupConnections();
  /*! \brief Initializes ui->mod_list and ui->deployer_list and creates connections. */
  void setupLists();
  /*! \brief Initializes context menus. */
  void setupMenus();
  /*! \brief Initializes all reusable dialogs. */
  void setupDialogs();
  /*!
   * \brief Updates ui->mod_list with new data.
   * \param mod_info Contains information about all mods belonging to the
   * current ModdedApplication.
   */
  void updateModList(const std::vector<ModInfo>& mod_info);
  /*!
   * \brief Updates ui->mod_list with new data.
   * \param depl_info Contains information about all mods belonging to all
   * \ref Deployer "deployers".
   */
  void updateDeployerList(const DeployerInfo& depl_info);
  /*!
   * \brief Returns the currently active ModdedApplication.
   * \return The active ModdedApplication.
   */
  int currentApp();
  /*!
   * \brief Returns the currently active Deployer.
   * \return The active Deployer.
   */
  int currentDeployer();
  /*!
   * \brief Returns the currently active profile.
   * \return The active profile.
   */
  int currentProfile();
  /*! \brief Applies current search filter to ui->mod_list. */
  void filterModList();
  /*! \brief Applies current search and conflicts filters to ui->deployer_list. */
  void filterDeployerList();
  /*! \brief Initializes all buttons belonging to this window. */
  void setupButtons();
  /*!
   * \brief Initializes and shows a dialog used to edit the given deployer.
   * \param deployer Target deployer.
   */
  void showEditDeployerDialog(int deployer);
  /*!
   * \brief Extracts or downloads the next mod in mod_import_targets_.
   */
  void importMod();
  /*!
   * \brief Sets the visibility status of the progress bar and disabled various UI elements while
   * the \ref ApplicationManager is busy.
   * \param busy The new status.
   * \param show_progress_bar If true: Show the progress bar if busy == true.
   * \param disable_app_launch If true: Disable the run app action.
   */
  void setBusyStatus(bool busy, bool show_progress_bar = true, bool disable_app_launch = false);
  /*!
   * \brief Returns the index of the given column name for the given QTableWidget.
   * \param table Target table.
   * \param col_name Name of the column.
   * \return The index or -1 if none was found.
   */
  int getColumnIndex(QTableWidget* table, QString col_name);
  /*!
   * \brief Sets a message for the status bar.
   * \param message New status message.
   * \param timeout Length in milliseconds for which the message will be shown.
   * If == 0: Show message indefinitely.
   */
  void setStatusMessage(QString message, int timeout_ms = 0);
  /*! \brief Initializes the log frame and button. */
  void setupLog();
  /*!
   * \brief Runs the given command and returns its output and return code.
   * \param command Command to be run.
   * \param ignore_flatpak If true: Do not modify the command for use inside of a flatpak sandbox.
   * \return Output, return code
   */
  QPair<QString, int> runCommand(QString command, bool ignore_flatpak = false);
  /*!
   * \brief Runs the given command in a separate thread and prints its output to the log.
   * \param command Command to be run.
   * \param name Name of the command.
   * \param type Type of command, e.g. 'Tool'.
   * \param ignore_flatpak If true: Do not modify the command for use inside of a flatpak sandbox.
   */
  void runConcurrent(QString command, QString name, QString type, bool ignore_flatpak = false);
  /*!
   * \brief Loads all stored settings from the settings file and updates the respective values.
   */
  void loadSettings();
  /*! \brief Generates and sets the style sheet for ui->app_tab_widget. */
  void setTabWidgetStyleSheet();
  /*!
   * \brief Returns a vector of bools indicating for each \ref Deployer "deployer"
   * if that deployer is autonomous.
   * \return The vector.
   */
  std::vector<bool> getAutonomousDeployers();
  /*!
   * \brief Enables/ Disables actions which allow for the removal, adding or editing
   * of applications.
   * \param enabled The new status for the actions.
   */
  void enableModifyApps(bool enabled);
  /*!
   * \brief Enables/ Disables actions which allow for the removal, adding or editing
   * of deployers and apps.
   * \param enabled The new status for the actions.
   */
  void enableModifyDeployers(bool enabled);
  /*!
   * \brief Enables/ Disables actions which allow for the removal, adding or editing
   * of backups and backup targets.
   * \param enabled The new status for the actions.
   */
  void enableModifyBackups(bool enabled);
  /*!
   * \brief Enables/ Disables actions which allow for the removal, adding or editing
   * of profiles and applications.
   * \param enabled The new status for the actions.
   */
  void enableModifyProfiles(bool enabled);
  /*!
   * \brief Initializes the NexusMods API key from the values stored in the settings file.
   * \return True if initialization was successful.
   */
  bool initNexusApiKey();
  /*! \brief Initializes the QLocalServer used for communications with other Limo instances. */
  void setupIpcServer();
  /*!
   * \brief Disables/ enables UI elements in case no \ref ModdedApplication "applications" exist.
   * \param has_apps If false: Disables various elements used to edit apps, else: Enable the
   * elements.
   */
  void initUiWithoutApps(bool has_apps);
  /*! \brief Checks the environmet for signs that Limo is running in a container, like flatpak. */
  void checkForContainers();
  /*! \brief Checks the current settings for outdated entries and adepts those as needed. */
  void updateOutdatedSettings();
  /*!
   * \brief Compares two version strings by priority according to their position in the string.
   * Version strings must contain only numbers and ".". Earlier numbers have higher priority,
   * separated by "." chatacers. Example: 20.5.4 > 2.1.4
   * \param current_version Version for comparison.
   * \param target_version Current version has to be <= this.
   * \return True if current_version <= target_version, else false. False if target_version
   * is invalid, true if current_version is invalid.
   */
  bool versionIsLessOrEqual(QString current_version, QString target_version);
  /*! \brief Initializes the root level conditions for the current app. */
  void initRootLevelConditions();

public slots:
  /*!
   * \brief Called when files are dragged into the application. Stores paths to all files
   * and begins installation.
   * \param paths Paths to added files.
   */
  void onModAdded(QList<QUrl> paths);
  /*!
   * \brief Installs a new mod.
   * \param app_id Application for which the new mod is to be installed.
   * \param info Contains all data needed to install the mod.
   */
  void onAddModDialogAccept(int app_id, ImportModInfo info);
  /*!
   * \brief Called when a mod gets disabled/ enabled in ui->deployer_list.
   * Updates the mods state.
   * \param mod_id Target mod.
   * \param status New mod status.
   */
  void onDeployerBoxChange(int mod_id, bool status);
  /*!
   * \brief Calls \ref updateModList with new data.
   * \param mod_info The new data displayed in ui->mod_list.
   */
  void onGetModInfo(std::vector<ModInfo> mod_info);
  /*!
   * \brief Calls \ref updateDeployerList with new data.
   * \param mod_info The new data displayed in ui->deployer_list.
   */
  void onGetDeployerInfo(DeployerInfo depl_info);
  /*!
   * \brief Adds a new \ref ModdedApplication "application".
   * \param info Contains all data entered in the dialog.
   */
  void onAddAppDialogComplete(EditApplicationInfo info);
  /*!
   * \brief Adds a new Deployer.
   * \param info Contains all data entered in the dialog.
   * \param app_id Target application.
   */
  void onAddDeployerDialogComplete(EditDeployerInfo info, int app_id);
  /*!
   * \brief Updates the application combo box with new ModdedApplication names.
   * \param names The new names.
   * \param icon_paths Paths to application icons.
   * \param is_new Indicates if this was called after adding a new application.
   */
  void onGetApplicationNames(QStringList names, QStringList icon_paths, bool is_new);
  /*!
   * \brief Updates the Deployer combo box with new deployer names.
   * \param names The new names.
   * \param is_new Indicates if this was called after adding a new deployer.
   */
  void onGetDeployerNames(QStringList names, bool is_new);
  /*!
   * \brief Called on right clicking in ui->mod_list. Shows a context menu
   * at the given position.
   * \param pos Target position for menu.
   */
  void onModListContextMenu(QPoint pos);
  /*!
   * \brief Called on right clicking in ui->deployer_list. Shows a context menu
   * at the given position.
   * \param pos Target position for menu.
   */
  void onDeployerListContextMenu(QPoint pos);
  /*!
   * \brief Updates which deployers should manage given mods.
   * \param mod_id Target mods.
   * \param deployers Bool for every deployer, indicating if the mods should be managed
   * by that deployer.
   */
  void onAddToDeployerAccept(std::vector<int>& mod_ids, std::vector<bool> deployers);
  /*!
   *  \brief Hides the progress bar and shows the given status message.
   *  \param message Status message to show.
   */
  void onCompletedOperations(QString message = "");
  /*!
   * \brief Shows a ConflictsWidget window containing given file conflict information.
   * \param conflicts Conflicts to be shown.
   */
  void onGetFileConflicts(std::vector<ConflictInfo> conflicts);
  /*!
   * \brief Updates the "App" tab.
   * \param app_info New data used for the update.
   */
  void onGetAppInfo(AppInfo app_info);
  /*!
   * \brief Updates an \ref ModdedApplication "application" and optionally
   * moves all of it's mods to a new directory.
   * \param info Contains all data entered in the dialog.
   * \param app_id The target \ref ModdedApplication "application".
   */
  void onApplicationEdited(EditApplicationInfo info, int app_id);
  /*!
   * \brief Updates type, name and target directory for one deployer of one
   * \ref ModdedApplication "application".
   * \param info Contains all data entered in the edit dialog.
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer Target Deployer.
   */
  void onDeployerEdited(EditDeployerInfo info, int app_id, int deployer);
  /*!
   * \brief Filters ui->mod_list so that it only show conflicting mods.
   * \param conflicts Contains conflicting mod ids.
   */
  void onGetModConflicts(std::unordered_set<int> conflicts);
  /*!
   * \brief Moves a mod in the current load order.
   * \param from Original mod position.
   * \param to New mod position.
   */
  void onModMoved();
  /*!
   * \brief Updates the profile combo box with new profile names.
   * \param names The new names.
   * \param is_new Indicates if this was called after adding a new profile.
   */
  void onGetProfileNames(QStringList names, bool is_new);
  /*!
   * \brief Adds a new profile to the given \ref ModdedApplication "application"
   * and optionally copies it's load order from an existing profile.
   * \param app_id Target application
   * \param info Contains data for the new profile.
   */
  void onProfileAdded(int app_id, EditProfileInfo info);
  /*!
   * \brief Changes the name of the given profile.
   * \param app_id Target application
   * \param profile Target profile
   * \param info Contains the new data for the profile.
   */
  void onProfileEdited(int app_id, int profile, EditProfileInfo info);
  /*!
   * \brief Deletes the currently active profile.
   */
  void onProfileRemoved();
  /*!
   * \brief Adds the given mod to the given group.
   * \param mod_id Target mod.
   * \param target_id Target group.
   */
  void onModAddedToGroup(int mod_id, int target_id);
  /*!
   * \brief Called when "cancel" button is pressed during mod installation.
   * Clears all pending mod installations.
   * \param app_id Path to the directory used for extracting the mod archive.
   */
  void onAddModAborted(QString temp_dir);
  /*!
   * \brief Called when a mod has been moved to a different position using the MoveModDialog
   * \param from Original position.
   * \param to New position.
   */
  void onModMovedTo(int from, int to);
  /*!
   * \brief Called after archive has been extracted. Installs the newly extracted mod.
   * \param info Contains data for the extracted mod.
   */
  void onExtractionComplete(ImportModInfo info);
  /*! \brief Called when the settings dialog has completed. Updates state with new settings. */
  void onSettingsDialogComplete();
  /*!
   * \brief Updates the backups tab with new data.
   * \param Contains information about all managed backup targets.
   */
  void onGetBackupInfo(std::vector<BackupTarget> backups);
  /*!
   * \brief Called when the \ref AddBackupTargetDialog has been completed.
   * \param app_id Application to which the new backup target is to be added.
   * \param name Name of the new backup target.
   * \param path Path to the file or directory to be managed.
   * \param default_backup Name of the currently active version of the target.
   * \param first_backup If not empty: Name of the first backup.
   */
  void onBackupTargetAdded(int app_id,
                           QString name,
                           QString path,
                           QString default_backup,
                           QString first_backup);
  /*!
   * \brief Called on right clicking in ui->backup_list. Shows a context menu
   * at the given position.
   * \param pos Target position for menu.
   */
  void onBackupListContextMenu(QPoint pos);
  /*!
   * \brief Called when the AddBackupDialog has been accepted. Adds a new backup to an
   * existing target.
   * \param app_id Target app.
   * \param target Target for which to add a new backup.
   * \param name Backup name.
   * \param target_name Name of the target for which to create a new backup.
   * \param source Backup from which to copy files to create the new backup. If -1:
   * copy currently active backup.
   */
  void onBackupAdded(int app_id, int target, QString name, QString target_name, int source);
  /*! \brief Resizes the mod list columns to fit the current contents. */
  void resizeModListColumns();
  /*! \brief Resizes the deployer list columns to fit the current contents. */
  void resizeDeployerListColumns();
  /*! \brief Initializes the progress bar and adds it to the status bar. */
  void setupProgressBar();
  /*! \brief Initializes ui elements used to filter lists. */
  void setupFilters();
  /*! \brief Initializes some ui icons. */
  void setupIcons();

private slots:
  /*! \brief Deploys all mods for the currently active ModdedApplication. */
  void on_deploy_button_clicked();
  /*! \brief Adds a separator to the deployer for the currently active ModdedApplication. */
  void on_deployer_add_separator_button_clicked();
  /*! \brief Shows a dialog to add a new ModdedApplication. */
  void onAddAppButtonClicked();
  /*! \brief Updates the currently active ModdedApplication. */
  void on_app_selection_box_currentIndexChanged(int index);
  /*! \brief Shows a dialog to add a new Deployer. */
  void onAddDeployerButtonClicked();
  /*! \brief Updates the currently active Deployer. */
  void on_deployer_selection_box_currentIndexChanged(int index);
  /*! \brief Deletes the currently active Deployer. */
  void onRemoveDeployerButtonClicked();
  /*! \brief Deletes the currently active ModdedApplication. */
  void onRemoveAppButtonClicked();
  /*! \brief Shows a dialog to add the currently selected mod to a Deployer. */
  void on_actionadd_to_deployer_triggered();
  /*! \brief Removes the currently selected mod from the current Deployer. */
  void on_actionremove_from_deployer_triggered();
  /*! \brief Generates file conflicts for currently selected mod. */
  void on_actionget_file_conflicts_triggered();
  /*! \brief Shows a dialog to edit the currently active Deployer. */
  void onEditDeployerPressed();
  /*! \brief Shows a dialog to add a new tool. */
  void onAddToolClicked();
  /*! \brief Launches the current application. */
  void onLaunchAppButtonClicked();
  /*! \brief Shows a dialog to edit the currently active ModdedApplication. */
  void on_edit_app_button_clicked();
  /*! \brief Applies the search field text as a filter to ui->mod_list and ui->deployer_list. */
  void on_search_field_textEdited(const QString& text);
  /*!
   *  \brief Filters mods such that only those in conflict with the currently
   *  selected mod are visible .
   */
  void on_actionget_mod_conflicts_triggered();
  /*! \brief Resets filters such that lists show all mods. */
  void on_reset_filter_button_clicked();
  /*!
   *  \brief Shows a dialog to move the currently selected mod to
   *  a new position in the load order
   */
  void on_actionmove_mod_triggered();
  /*! \brief Shows a dialog to edit the currently active Deployer. */
  void onEditDeployerMenuClicked();
  /*! \brief Updates the currently active profile. */
  void on_profile_selection_box_currentIndexChanged(int index);
  /*! \brief Shows a dialog to add a new profile. */
  void onAddProfileButtonClicked();
  /*! \brief Shows a dialog to edit the currently active profile. */
  void onEditProfileButtonClicked();
  /*! \brief Removes the currently active profile. */
  void onRemoveProfileButtonClicked();
  /*!
   * \brief Shows a message box displaying an error.
   * \param title Window title for the message box.
   * \param message Message to be displayed.
   */
  void onReceiveError(QString title, QString message);
  /*! \brief Adds a dialog to add the currently selected mod to a group. */
  void on_actionAdd_to_Group_triggered();
  /*! \brief Removes the currently selected mod from its group. */
  void on_actionRemove_from_Group_triggered();
  /*! \brief Opens the file explorer at the staging directory of the selected mod. */
  void on_actionbrowse_mod_files_triggered();
  /*! \brief Opens the file explorer at the target directory of the selected deployer. */
  void on_actionbrowse_deployer_files_triggered();
  /*! \brief Sorts and colors mods by conflicts groups. */
  void on_actionSort_Mods_triggered();
  /*! \brief Toggles log window visibility. */
  void onLogButtonPressed();
  /*!
   * \brief Shows the received message in the log.
   * \param log_level Log level for the message.
   * \param message Message content.
   */
  void onReceiveLogMessage(Log::LogLevel log_level, QString message);
  /*!
   * \brief Notifies app_manager_ of version change.
   * \param mod_id Target mod id.
   * \param version New version.
   */
  void onModVersionEdited(int mod_id, QString version);
  /*!
   * \brief Notifies app_manager_ of active group member change.
   * \param group Target group.
   * \param mod_id New active member.
   */
  void onActiveGroupMemberChanged(int group, int mod_id);
  /*!
   * \brief Notifies app_manager_ to change the name of the given mod.
   * \param mod_id If of target mod.
   * \param name New name.
   */
  void onModNameChanged(int mod_id, QString name);
  /*!
   *  \brief Notifies app_manager_ to remove the mod in the currently selected mod.
   *  \param mod_id Id of mod which is to be removed.
   *  \param name Name of mod which is to be removed.
   */
  void onModRemoved(int mod_id, QString name);
  /*! \brief Shows the settings dialog. */
  void on_settings_button_clicked();
  /*! \brief Called when the add backup target button has been clicked.*/
  void onAddBackupTargetClicked();
  /*!
   * \brief Called when the index of a backup combo box changes. Updates the active backup.
   * \param target Backup target.
   * \param backup New active backup.
   */
  void onActiveBackupChanged(int target, int backup);
  /*!
   * \brief Called when the name of a backup was edited by the user.
   * \param target Backup target.
   * \param backup Edited backup.
   * \param name The new name.
   */
  void onBackupNameEdited(int target, int backup, QString name);
  /*!
   * \brief Called when a remove backup target button has been clicked.
   * \param target Backup target to be removed.
   * \param name Name of the target.
   */
  void onBackupTargetRemoveClicked(int target, QString name);
  /*!
   * \brief Called when the name of a backup target was edited by the user.
   * \param target Backup target.
   * \param name The new name.
   */
  void onBackupTargetNameEdited(int target, QString name);
  /*! \brief Adds a new backup to the currently selected target. */
  void on_actionAdd_Backup_triggered();
  /*! \brief Removes the currently active backup from the currently selected target. */
  void on_actionRemove_Backup_triggered();
  /*!
   * \brief Disabled or enables some actions based on the current tab.
   * \param index New index.
   */
  void on_app_tab_widget_currentChanged(int index);
  /*! \brief Opens the default file manager at the location of the selected backup. */
  void on_actionBrowse_backup_files_triggered();
  /*! \brief Opens a OverWriteBackupDialog. */
  void on_actionOverwrite_Backup_triggered();
  /*!
   * \brief Emits \ref overwriteBackup.
   * \param target_id Id of the backup target for which to overwrite a backup.
   * \param source_backup Backup from which to copy files.
   * \param dest_backup Backup to be overwritten.
   */
  void onBackupOverwritten(int target_id, int source_backup, int dest_backup);
  /*!
   * \brief Used to synchronize scrolling in lists with the event queue. Scrolls
   * deployer and mod list to their currently selected index.
   */
  void onScrollLists();
  /*!
   * \brief If progress bar is visible: Update it with the given progress.
   * \param progress The new progress.
   */
  void updateProgress(float progress);
  /*!
   * \brief Removes all selected mods.
   */
  void on_actionRemove_Mods_triggered();
  /*! \brief Removes all group members of the currently selected mod, except for the active one. */
  void on_actionRemove_Other_Versions_triggered();
  /*!
   * \brief Enables actions for modifying apps.
   * \param return_code Indicates whether the dialog was completed successfully.
   */
  void onAddAppDialogFinished(int return_code);
  /*!
   * \brief Enables actions for modifying deployers and apps.
   * \param return_code Indicates whether the dialog was completed successfully.
   */
  void onAddDeployerDialogFinished(int return_code);
  /*!
   * \brief Enables actions for modifying backups and apps.
   * \param return_code Indicates whether the dialog was completed successfully.
   */
  void onAddBackupTargetDialogFinished(int return_code);
  /*!
   * \brief Called when a dialog, which requires the busy status to be set, was aborted.
   * Sets busy status to false.
   * \param return_code Indicates whether the dialog was completed successfully.
   */
  void onBusyDialogAborted();
  /*!
   * \brief Enables actions for modifying profiles and apps.
   * \param return_code Indicates whether the dialog was completed successfully.
   */
  void onAddProfileDialogFinished(int return_code);
  /*! \brief Toggles visibility of the filters widget. */
  void on_filters_button_clicked();
  /*!
   * \brief Adds/ removes the inactive mods filter to the mod list proxy.
   * \param state The checkbox state. If this is unchecked, the relevant filters will be removed.
   * If this is checked, only inactive mods will be shown.
   * If this is partially checked, only active mods will be shown.
   */
  void on_filter_active_mods_cb_stateChanged(int state);
  /*!
   * \brief Adds/ removes the mods in groups filter to the mod list proxy.
   * \param state The checkbox state. If this is unchecked, the relevant filters will be removed.
   * If this is checked, only mods without groups will be shown.
   * If this is partially checked, only mods with groups will be shown.
   */
  void on_filter_group_mods_cb_stateChanged(int state);
  /*!
   * \brief Adds/ removes the mods in groups filter to the deployer list proxy.
   * \param state The checkbox state. If this is unchecked, the relevant filters will be removed.
   * If this is checked, only inactive mods will be shown.
   * If this is partially checked, only active mods will be shown.
   */
  void on_filter_active_mods_depl_cb_stateChanged(int state);
  /*! \brief Opens the EditManualTagsDialog window. */
  void on_edit_manual_tags_button_clicked();
  /*!
   * \brief Called by the edit_manual_tags_dialog_ when editing is completed.
   * \param app_id Target app.
   * \param actions Contains all editing actions to be performed.
   */
  void onManualTagsEdited(int app_id, std::vector<EditManualTagAction> actions);
  /*!
   * \brief Updates the manual tags for all given mods.
   * \param app_id Target app.
   * \param tags Tags to be applied to the given mods. All other tags are removed.
   * \param mod_ids Mods the tags of which are to be updated.
   * \param mode Indicates whether tags should be added, removed or overwritten.
   */
  void onManualModTagsUpdated(int app_id, QStringList tags, std::vector<int> mod_ids, int mode);
  /*! \brief Opens the manage_manual_tags_dialog_ for all currently selected mods. */
  void on_actionEdit_Tags_for_mods_triggered();
  /*!
   * \brief Updates the mod_list filter with the new state for the given tag.
   * \param tag Manual tag used as filter.
   * \param state New check box state. If == Qt::Checked, tag is filtered out,
   * if == Qt::PartiallyChecked only mods with this tags are shown.
   */
  void onModManualTagFilterChanged(QString tag, int state);
  /*!
   * \brief Updates the deployer_list filter with the new state for the given tag.
   * \param tag Tag used as filter.
   * \param state New check box state. If == Qt::Checked, tag is filtered out,
   * if == Qt::PartiallyChecked only mods with this tags are shown.
   */
  void onDeplTagFilterChanged(QString tag, int state);
  /*! \brief Opens the EditAutoTagsDialog. */
  void on_edit_auto_tags_button_clicked();
  /*!
   * \brief Called by the edit_auto_tags_dialog_ when editing is completed.
   * \param app_id Target app.
   * \param actions Contains all editing actions to be performed.
   */
  void onAutoTagsEdited(int app_id, std::vector<EditAutoTagAction> actions);
  /*! \brief Reapplies all auto tags. */
  void on_update_auto_tags_button_clicked();
  /*! \brief Reapplies tags to the currently selected mods. */
  void on_actionUpdate_Tags_triggered();
  /*! \brief Shows the EditModSourcesDialog for the currently selected mod. */
  void on_actionEdit_Mod_Sources_triggered();
  /*!
   * \brief Updates local and remote mod sources for the given mod.
   * \param app_id App to which the edited mod belongs.
   * \param mod_id Target mod id.
   * \param local_source Path to a local archive or directory used for mod installation.
   * \param remote_source Remote URL from which the mod was downloaded.
   */
  void onModSourcesEdited(int app_id, int mod_id, QString local_source, QString remote_source);
  /*! \brief Shows data from NexusMods for the currently selected mod. */
  void on_actionShow_Nexus_Page_triggered();
  /*!
   * \brief Shows a NexusModDialog with data from the given page.
   * \param page Page with mod data.
   */
  void onGetNexusPage(int app_id, int mod_id, nexus::Page page);
  /*!
   * \brief Called by the IPC server when a message from another Limo instance was received.
   * If the message contains a nexus mod nxm link: Downloads the respective mod.
   * \param message The message.
   */
  void onReceiveIpcMessage(QString message);
  /*!
   * \brief Begins extraction of downloaded mod.
   * \param info Contains all relevant data for the extraction process.
   */
  void onDownloadComplete(ImportModInfo info);
  /*!
   * \brief Downloads a mod from nexusmods using the given mod_url. Only works if the given
   * api key belongs to a premium user.
   * \param app_id App for which the mod is to be downloaded. The mod is downloaded to the apps
   * staging directory.
   * \param mod_id Id of the mod for which the file is to be downloaded. This is the limo internal
   * mod id, NOT the NexusMods id.
   * \param file_id NexusMods file id of the target file.
   * \param mod_url Url to the NexusMods page of the mod.
   * \param version If not empty: Use this to overwrite the default version.
   */
  void onModDownloadRequested(int app_id,
                              int mod_id,
                              int file_id,
                              QString mod_url,
                              QString version);
  /*! \brief Cancels installation of the pending mod. */
  void onDownloadFailed();
  /*! \brief Reinstalls the currently selected mod from the local source. */
  void on_actionReinstall_From_Local_triggered();
  /*! \brief Checks for updates for all mods for the current application. */
  void on_check_mod_updates_button_clicked();
  /*! \brief Selects all mods in the mods_tab. */
  void on_actionSelect_All_triggered();
  /*!
   * \brief Adds a filter to the mod list for updated mods depending on the new check sate.
   * \param state New check state. Filters for mods with updates if this is checked,
   * filters for mods without updates if partially checked and removes the filter in unchecked.
   */
  void on_filter_mods_with_updates_cb_stateChanged(int state);
  /*! \brief Checks for available updates for the selected mods. */
  void on_actionCheck_For_Updates_triggered();
  /*! \brief Temporarily disables the update notification for all selected mods. */
  void on_actionSuppress_Update_triggered();
  /*!
   *  \brief Proceeds with the next mod import, if the queue is not empty.
   *  \param success If true: Installation was successful.
   */
  void onModInstallationComplete(bool success);
  /*!
   * \brief If at least one file has been changed: Show a ExternalChangesDialog.
   * Afterwards, get external changes for other deployers. If there are none: Begin deployment.
   * \param app_id \ref ModdedApplication "application" for which changes have been received.
   * \param info Contains data about externally modified files.
   * \param num_deployers The total number of deployers for the target app.
   * \param deploy If True: Deploy mods after checking, else: Undeploy mods.
   */
  void onGetExternalChangesInfo(int app_id,
                                ExternalChangesInfo info,
                                int num_deployers,
                                bool deploy);
  /*!
   * \brief Gets external changes for other deployers. If there are none: Begin deployment.
   * \param app_id \ref ModdedApplication "application" for which changes have benn handled.
   * \param deployer Deployer for which changes have been handled.
   * \param num_deployers The total number of deployers for the target app.
   * \param deploy If True: Deploy mods after checking, else: Undeploy mods.
   */
  void onExternalChangesHandled(int app_id, int deployer, int num_deployers, bool deploy);
  /*!
   * \brief Called when the ExternalChangesDialog has been completed sucessfully.
   * Emits \ref keepOrRevertFileModifications.
   * \param app_id Target app.
   * \param deployer Target deployer.
   * \param changes_to_keep Contains paths to modified files, the id of the mod currently
   * responsible for that file and a bool which indicates whether or not changes to
   * that file should be kept.
   * \param deploy If True: Deploy mods after checking, else: Undeploy mods.
   */
  void onExternalChangesDialogCompleted(int app_id,
                                        int deployer,
                                        const FileChangeChoices& changes_to_keep,
                                        bool deploy);
  /*! \brief Called when the ExternalChangesDialog has been aborted. Cancels deployment. */
  void onExternalChangesDialogAborted();
  /*! \brief Opens the export_app_config_dialog_. */
  void on_export_app_config_button_clicked();
  /*!
   * \brief Emits \ref exportAppConfiguration.
   * \param app_id Target app.
   * \param deployers Deployers to export.
   * \param auto_tags Auto tags to export.
   */
  void onExportAppConfigDialogComplete(int app_id,
                                       std::vector<int> deployers,
                                       QStringList auto_tags);
  /*! \brief Checks for external changes and reverts mod deployment. */
  void on_undeploy_button_clicked();
  /*!
   * \brief Updates the file ignore list for ReverseDeployers
   * \param app_id Target app.
   * \param deployer Target deployer.
   */
  void onUpdateIgnoredFiles(int app_id, int deployer);
  /*!
   * \brief Adds the currently selected mod in the deployer list to the ignore list of the
   * current ReverseDeployer.
   */
  void on_actionAdd_to_Ignore_List_triggered();
  /*!
   * \brief Launches the tool to which this button belongs.
   * \param row Row of the button in the tools table.
   * \param col Column of the button in the tools table.
   */
  void onLaunchToolButtonPressed(int row, int col);
  /*!
   * \brief Edits the tool to which this button belongs.
   * \param row Row of the button in the tools table.
   * \param col Column of the button in the tools table.
   */
  void onEditToolButtonPressed(int row, int col);
  /*!
   * \brief Deletes the tool to which this button belongs.
   * \param row Row of the button in the tools table.
   * \param col Column of the button in the tools table.
   */
  void onRemoveToolButtonPressed(int row, int col);
  /*!
   * \brief Adds a new tool with the given data.
   * \param app_id App to which to add the tool.
   * \param tool The new tool.
   */
  void onToolAdded(int app_id, Tool tool);
  /*!
   * \brief Replaces the given tool with the given new tool.
   * \param app_id App to which to add the tool.
   * \param tool_id Target tool.
   * \param tool The new tool.
   */
  void onToolEdited(int app_id, int tool_id, Tool tool);
  /*!
   * \brief Emits \ref applyModAction for the given action.
   * \param action Action to be applied to the current mod.
   */
  void onModActionTriggered(int action);

signals:
  /*!
   * \brief Creates a vector containing information about all installed mods, stored in ModInfo
   * objects for one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   */
  void getModInfo(int app_id);
  /*!
   * \brief Adds a new \ref ModdedApplication "application".
   * \param info Contains all data needed to add a new application, e.g. its name.
   */
  void addApplication(EditApplicationInfo info);
  /*!
   * \brief Adds a new Deployer of given type to given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param info Contains all data needed to create a new deployer, e.g. its name.
   */
  void addDeployer(int app_id, EditDeployerInfo info);
  /*!
   * \brief Creates DeployerInfo for one Deployer for one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer Target deployer.
   */
  void getDeployerInfo(int app_id, int deployer);
  /*!
   * \brief Installs a new mod for one \ref ModdedApplication "application" using
   * the given Installer type.
   * \param app_id The target \ref ModdedApplication "application".
   * \param info Contains all data needed to install the mod.
   */
  void installMod(int app_id, ImportModInfo info);
  /*!
   * \brief Uninstalls the given mods for one \ref ModdedApplication "application", this includes
   * deleting all installed files.
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_ids Ids of the mods to be uninstalled.
   * \param installer_type The Installer type used. If an empty string is given, the Installer
   * used during installation is used.
   */
  void uninstallMods(int app_id, std::vector<int> mod_ids, std::string installer_type);
  /*!
   * \brief Updates which \ref Deployer "deployer" should manage given mods.
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_id Vector of mod ids to be added.
   * \param deployers Bool for every deployer, indicating if the mods should be managed
   * by that deployer.
   */
  void updateModDeployers(int app_id, std::vector<int> mod_ids, std::vector<bool> deployers);
  /*!
   * \brief Removes a mod from the load order for given Deployer for
   * given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer The target Deployer
   * \param mod_id Id of the mod to be removed.
   */
  void removeModFromDeployer(int app_id, int deployer, void *node_ptr);
  /*!
   * \brief Enables or disables the given mod in the load order for given Deployer
   * for given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer The target Deployer
   * \param mod_id Mod to be edited.
   * \param status The new status.
   */
  void setModStatus(int app_id, int deployer, int mod_id, bool status);
  /*!
   * \brief Moves a mod from one position in the load order to another for given Deployer
   * for given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer The target Deployer.
   * \param from_index Index of mod to be moved.
   * \param to_index Destination index.
   */
  void changeLoadorder(int app_id, int deployer, int from_idx, int to_idx);
  void commitChanges(int app_id, int deployer);
  /*!
   * \brief Deploys mods using all Deployer objects of one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   */
  void deployMods(int app_id);
  /*!
   * \brief Deploys mods using given Deployers of one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer_ids Target Deployer ids.
   */
  void deployModsFor(int app_id, std::vector<int> deployer_ids);
  /*!
   * \brief Undeploys mods using all Deployer objects of one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   */
  void unDeployMods(int app_id);
  /*!
   * \brief Undeploys mods using given Deployers of one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer_ids Target Deployer ids.
   */
  void unDeployModsFor(int app_id, std::vector<int> deployer_ids);
  /*!
   * \brief Updates Entries in the \ref ModdedApplication "application" combo box.
   * \param is_new Indicates whether this was called after adding a
   * new \ref ModdedApplication "application".
   */
  void getApplicationNames(bool is_new);
  /*!
   * \brief Updates Entries in the Deployer combo box.
   * \param app_id Target \ref ModdedApplication "application".
   * \param is_new Indicates whether this was called after adding a new Deployer.
   */
  void getDeployerNames(int app_id, bool is_new);
  /*!
   * \brief Removes a Deployer from an \ref ModdedApplication "application".
   * \param app_id Target \ref ModdedApplication "application".
   * \param deployer Target Deployer.
   * \param cleanup If true: Remove all currently deployed files and restore backups.
   */
  void removeDeployer(int app_id, int deployer, bool cleanup);
  /*!
   * \brief Removes an \ref ModdedApplication "application" and optionally
   * deletes all installed mods and the settings file in the
   * \ref ModdedApplication "application's" staging directory.
   * \param app_id The target \ref ModdedApplication "application".
   * \param cleanup Indicates if mods and settings file should be deleted.
   */
  void removeApplication(int app_id, bool cleanup);
  /*!
   * \brief Setter for a mod name.
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_id Target mod.
   * \param new_name The new name.
   */
  void changeModName(int app_id, int mod_id, QString new_name);
  /*!
   * \brief Checks for file conflicts of given mod with all other mods in the load order for
   * one Deployer of one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer The target Deployer
   * \param mod_id Mod to be checked.
   * \param show_disabled If true: Also check for conflicts with disabled mods.
   */
  void getFileConflicts(int app_id, int deployer, int mod_id, bool show_disabled);
  /*!
   * \brief Creates AppInfo for given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   */
  void getAppInfo(int app_id);
  /*!
   * \brief Adds a new tool to given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param tool The new tool.
   */
  void addTool(int app_id, Tool tool);
  /*!
   * \brief Removes a tool from given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param tool_id The tool's id.
   */
  void removeTool(int app_id, int tool_id);
  /*!
   * \brief Edits an \ref ModdedApplication "application" and optionally moves all
   *  of it's mods to a new directory.
   * \param info Contains all data needed to edit the application, e.g. its new name.
   * \param app_id The target \ref ModdedApplication "application".
   */
  void editApplication(EditApplicationInfo info, int app_id);
  /*!
   * \brief Used to set type, name and target directory for one deployer of one
   * \ref ModdedApplication "application".
   * \param info Contains all data needed to edit a deployer, e.g. its new name.
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer Target Deployer.
   */
  void editDeployer(EditDeployerInfo info, int app_id, int deployer);
  /*!
   * \brief Checks for conflicts with other mods for one Deployer of
   * one \ref ModdedApplication "application".
   * Two mods are conflicting if they share at least one file.
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer Target Deployer.
   * \param mod_id The mod to be checked.
   */
  void getModConflicts(int app_id, int deployer, int mod_id);
  /*!
   * \brief Sets the currently active profile for given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param profile The new profile.
   */
  void setProfile(int app_id, int profile);
  /*!
   * \brief Adds a new profile to one \ref ModdedApplication "application" and optionally
   * copies it's load order from an existing profile.
   * \param app_id The target \ref ModdedApplication "application".
   * \param info Contains data for the new profile.
   */
  void addProfile(int app_id, EditProfileInfo info);
  /*!
   * \brief Removes a profile from an \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param profile The profile to be removed.
   */
  void removeProfile(int app_id, int profile);
  /*!
   * \brief Creates a vector containing the names of all profiles of one
   * \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   */
  void getProfileNames(int app_id, bool is_new);
  /*!
   * \brief Used to set the name of a profile for one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param profile Target Profile
   * \param info Contains the new data for the profile.
   */
  void editProfile(int app_id, int profile, EditProfileInfo info);
  /*!
   * \brief Used to replace an existing to with a now one for a \ref ModdedApplication
   * "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param tool_id Target tool.
   * \param new_tool The new tool.
   */
  void editTool(int app_id, int tool_id, Tool new_tool);
  /*!
   * \brief Adds a mod to an existing group of an \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_id The mod's id.
   * \param group The target group.
   */
  void addModToGroup(int app_id, int mod_id, int group);
  /*!
   * \brief Removes a mod from it's group for one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_id Target mod.
   */
  void removeModFromGroup(int app_id, int mod_id);
  /*!
   * \brief Creates a new group containing the two given mods for one
   * \ref ModdedApplication "application". A group is a set of mods where only one member,
   * the active member, will be deployed.
   * \param app_id The target \ref ModdedApplication "application".
   * \param first_mod_id First mod. This will be the active member of the new group.
   * \param second_mod_id Second mod.
   */
  void createGroup(int app_id, int first_mod_id, int second_mod_id);
  /*!
   * \brief Changes the active member of given group of an
   * \ref ModdedApplication "application" to given mod.
   * \param app_id The target \ref ModdedApplication "application".
   * \param group Target group.
   * \param mod_id The new active member.
   */
  void changeActiveGroupMember(int app_id, int group, int mod_id);
  /*!
   * \brief Sets the given mod's version to the given new version for
   * one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_id Target mod.
   * \param new_version The new version.
   */
  void changeModVersion(int app_id, int mod_id, QString new_version);
  /*!
   * \brief Sorts the load order by grouping mods which contain conflicting files.
   * \param app_id Target app.
   * \param deployer Target deployer.
   */
  void sortModsByConflicts(int app_id, int deployer);
  /*!
   * \brief Extracts an archive to target directory.
   * \param info Contains all data needed to extract the mod archive.
   */
  void extractArchive(ImportModInfo info);
  /*!
   * \brief Requests info about backups for one ModdedApplication.
   * \param app_id Target app.
   */
  void getBackupInfo(int app_id);
  /*!
   * \brief Adds a new target file or directory to be managed by the BackupManager of given
   * ModdedApplication.
   * \param app_id Target app.
   * \param path Path to the target file or directory.
   * \param name Display name for this target.
   * \param default_backup Display name for the currently active version of the target.
   * \param first_backup If not empty: Create a backup of the target with this as name.
   */
  void addBackupTarget(int app_id,
                       QString path,
                       QString name,
                       QString default_backup,
                       QString first_backup);
  /*!
   * \brief Removes the given backup target from the given ModdedApplication by deleting
   * all relevant backups and config files.
   * \param app_id Target app.
   * \param target_id Target to remove.
   */
  void removeBackupTarget(int app_id, int target_id);
  /*!
   * \brief Adds a new backup for the given target for the given ModdedApplication by copying
   * the currently active backup.
   * \param app_id Target app.
   * \param target_id Target for which to create a new backup.
   * \param name Display name for the new backup.
   * \param source Backup from which to copy files to create the new backup. If -1:
   * copy currently active backup.
   */
  void addBackup(int app_id, int target_id, QString name, int source);
  /*!
   * \brief Deletes the given backup for given target for given ModdedApplication.
   * \param app_id Target app.
   * \param target_id Target from which to delete a backup.
   * \param backup_id Backup to remove.
   */
  void removeBackup(int app_id, int target_id, int backup_id);
  /*!
   * \brief Changes the currently active backup for the given target for the given
   * ModdedApplication.
   * \param app_id Target app.
   * \param target_id Target for which to change the active backup.
   * \param backup_id New active backup.
   */
  void setActiveBackup(int app_id, int target_id, int backup_id);
  /*!
   * \brief Changes the name of the given backup for the given target for the given
   * ModdedApplication.
   * \param app_id Target app.
   * \param target_id Backup target.
   * \param backup_id Backup to be edited.
   * \param name The new name.
   */
  void setBackupName(int app_id, int target_id, int backup_id, QString name);
  /*!
   * \brief Changes the name of the given backup target for the given
   * ModdedApplication.
   * \param app_id Target app.
   * \param target_id Backup target.
   * \param name The new name.
   */
  void setBackupTargetName(int app_id, int target_id, QString name);
  /*!
   * \brief Deletes all files in the dest backup and replaces them with the files
   * from the source backup for the given ModdedApplication.
   * \param app_id Target app.
   * \param target_id Backup target.
   * \param source_backup Backup from which to copy files.
   * \param dest_backup Target for data deletion.
   */
  void overwriteBackup(int app_id, int target_id, int source_backup, int dest_backup);
  /*! \brief Used to synchronize scrolling in lists with the event queue. */
  void scrollLists();
  /*!
   * \brief Uninstalls all mods which are inactive group members of any group which contains
   * any of the given mods for the given ModdedApplication.
   * \param app_id Target app.
   * \param mod_ids Ids of the mods for which to uninstall group members.
   */
  void uninstallGroupMembers(int app_id, const std::vector<int>& mod_ids);
  /*!
   * \brief Signals that the given editing actions are to be performed on the manual tags of
   * the given ModdedApplication.
   * \param app_id Target app.
   * \param actions Editing actions.
   */
  void editManualTags(int app_id, std::vector<EditManualTagAction> actions);
  /*!
   * \brief Sets the tags for all given mods to the given tags for the given ModdedApplication.
   * \param app_id Target app.
   * \param tag_names Names of the new tags.
   * \param mod_ids Target mod ids.
   */
  void setTagsForMods(int app_id, QStringList tag_names, const std::vector<int> mod_ids);
  /*!
   * \brief Adds the given tag to all given mods for the given ModdedApplication.
   * \param app_id Target app.
   * \param tag_name Target tags name.
   * \param mod_ids Target mod ids.
   */
  void addTagsToMods(int app_id, QStringList tag_names, const std::vector<int>& mod_ids);
  /*!
   * \brief Removes the given tag from the given mods for the given ModdedApplication.
   * \param app_id Target app.
   * \param tag_name Target tags name.
   * \param mod_ids Target mod ids.
   */
  void removeTagsFromMods(int app_id, QStringList tag_names, const std::vector<int>& mod_ids);
  /*!
   * \brief Signals that the given editing actions are to be performed on the auto tags of
   * the given ModdedApplication.
   * \param app_id Target app.
   * \param actions Editing actions.
   */
  void editAutoTags(int app_id, std::vector<EditAutoTagAction> actions);
  /*!
   * \brief Reapplies all auto tags for all mods for the given ModdedApplication.
   * \param app_id Target app.
   */
  void reapplyAutoTags(int app_id);
  /*!
   * \brief Reapplies all auto tags to the given mods for the given ModdedApplication.
   * \param app_id Target app.
   * \param mod_ids Ids of the mods to which auto tags are to be reapplied.
   */
  void updateAutoTags(int app_id, std::vector<int> mod_ids);
  /*!
   * \brief Sets a mods local and remote source to the given values for the given ModdedApplication.
   * \param app_id App to which the edited mod belongs.
   * \param mod_id Target mod id.
   * \param local_source Path to a local archive or directory used for mod installation.
   * \param remote_source Remote URL from which the mod was downloaded.
   */
  void editModSources(int app_id, int mod_id, QString local_source, QString remote_source);
  /*!
   * \brief Fetches data for the given mod from NexusMods.
   * \param app_id App to which the mod belongs.
   * \param mod_id Target mod id.
   */
  void getNexusPage(int app_id, int mod_id);
  /*!
   * \brief Downloads a mod from remote using the given import info.
   * \param info Contains either the ImportModInfo::remote_request_url or
   * ImportModInfo::remote_mod_id and ImportModInfo::remote_file_id.
   */
  void downloadMod(ImportModInfo info);
  /*!
   * \brief Checks for available mod updates on NexusMods.
   * \param app_id App for which mod updates are to be checked.
   */
  void checkForModUpdates(int app_id);
  /*!
   * \brief Checks for available updates for the given mod for the given app.
   * \param app_id Target app.
   * \param mod_ids Ids of the mods for which to check for updates.
   */
  void checkModsForUpdates(int app_id, const std::vector<int>& mod_ids);
  /*!
   * \brief Temporarily disables update notifications for the given mods.
   * \param app_id Target app.
   * \param mod_ids Ids of the mods for which update notifications are to be disabled.
   */
  void suppressUpdateNotification(int app_id, const std::vector<int>& mod_ids);
  /*!
   * \brief Checks if files deployed by the given app by the given deployer have been
   * externally overwritten.
   * \param app_id Target app.
   * \param deployer Deployer to check.
   * \param deploy If True: Deploy mods after checking, else: Undeploy mods.
   */
  void getExternalChanges(int app_id, int deployer, bool deploy);
  /*!
   * \brief Keeps or reverts external changes for one app for one deployer.
   * For every given file: Moves the modified file into the source mods directory and links
   * it back in, if the changes are to be kept. Else: Deletes that file and restores
   * the original link.
   * \param app_id Target app.
   * \param deployer Target deployer.
   * \param modified_files Contains paths to modified files, the id of the mod currently
   * responsible for that file and a bool which indicates whether or not changes to
   * that file should be kept.
   * \param deploy If True: Deploy mods after checking, else: Undeploy mods.
   */
  void keepOrRevertFileModifications(int app_id,
                                     int deployer,
                                     const FileChangeChoices& changes_to_keep,
                                     bool deploy);
  /*!
   * \brief Exports configurations for the given deployers and the given auto tags to a json file.
   * Does not include mods.
   * \param app_id Target app.
   * \param deployers Deployers to export.
   * \param auto_tags Auto tags to export.
   */
  void exportAppConfiguration(int app_id, std::vector<int> deployers, QStringList auto_tags);
  /*!
   * \brief Updates the file ignore list for ReverseDeployers
   * \param app_id Target app.
   * \param deployer Target deployer.
   */
  void updateIgnoredFiles(int app_id, int deployer);
  /*!
   * \brief Adds the given mod to the ignore list of the given ReverseDeployer.
   * \param app_id Target app.
   * \param deployer Target deployer.
   * \param mod_id Mod to be ignored.
   */
  void addModToIgnoreList(int app_id, int deployer, int mod_id);
  /*!
   * \brief Applies the given mod action to the given mod.
   * \param app_id Target app.
   * \param deployer Target deployer.
   * \param action Action to be applied.
   * \param mod_id Target mod.
   */
  void applyModAction(int app_id, int deployer, int action, int mod_id);
};
