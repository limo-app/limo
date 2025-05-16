/*!
 * \file addappdialog.h
 * \brief Header for the AddAppDialog class.
 */

#pragma once

#include "../core/editapplicationinfo.h"
#include "../core/editdeployerinfo.h"
#include "importfromsteamdialog.h"
#include <json/json.h>


namespace Ui
{
class AddAppDialog;
}

/*!
 * \brief Dialog for creating and editing \ref ModdedApplication "applications".
 */
class AddAppDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit AddAppDialog(bool is_flatpak, QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~AddAppDialog();

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::AddAppDialog* ui;

  /*! \brief Name of the key used to identify deployers in the apps config file. */
  constexpr static inline std::string JSON_DEPLOYERS_GROUP = "deployers";
  /*! \brief Name of the key used to identify deployer type in the apps config file. */
  constexpr static inline std::string JSON_DEPLOYERS_TYPE = "type";
  /*! \brief Name of the key used to identify deployer name in the apps config file. */
  constexpr static inline std::string JSON_DEPLOYERS_NAME = "name";
  /*! \brief Name of the key used to identify deployer target dir in the apps config file. */
  constexpr static inline std::string JSON_DEPLOYERS_TARGET = "target_dir";
  /*! \brief Name of the key used to identify deployer mode in the apps config file. */
  constexpr static inline std::string JSON_DEPLOYERS_MODE = "deploy_mode";
  /*! \brief Name of the key used to identify deployer source dir in the apps config file. */
  constexpr static inline std::string JSON_DEPLOYERS_SOURCE = "source_dir";
  /*! \brief
   *  Name of the key used to determine whether a reverse deployer
   *  uses separate dirctories for profiles.
   */
  constexpr static inline char JSON_DEPLOYERS_SEPARATE_DIRS[] = "uses_separate_dirs";
  /*! \brief
   *  Name of the key used to determine whether a reverse deployer
   *  should update the ignore list upon creation.
   */
  constexpr static inline char JSON_DEPLOYERS_UPDATE_IGNORE_LIST[] = "update_ignore_list";
  /*! \brief Contains all mandatory valid keys used in a deployer group in the apps config file. */
  constexpr static std::array<std::string, 4> JSON_DEPLOYER_MANDATORY_KEYS{ JSON_DEPLOYERS_TYPE,
                                                                            JSON_DEPLOYERS_NAME,
                                                                            JSON_DEPLOYERS_TARGET,
                                                                            JSON_DEPLOYERS_MODE };
  /*! \brief Name of the key used to identify auto tags in the apps config file. */
  constexpr static inline std::string JSON_AUTO_TAGS_GROUP = "auto_tags";
  /*! \brief Name of the key used to identify the apps name in the apps config file. */
  constexpr static inline std::string JSON_NAME = "name";

  /*! \brief If true: Dialog is used to edit, else: Dialog is used to create. */
  bool edit_mode_ = false;
  /*! \brief Current name of the edited \ref ModdedApplication "application". */
  QString name_;
  /*! \brief Current staging directory path of the edited \ref ModdedApplication "application". */
  QString path_;
  /*! \brief Current command to run the edited \ref ModdedApplication "application". */
  QString command_;
  /*! \brief Id of the edited \ref ModdedApplication "application". */
  int app_id_;
  /*! \brief Steam app id of the new application, or -1 if not a steam app. */
  long steam_app_id_;
  /*! \brief Path to imported steam applications installation directory. */
  QString steam_install_path_ = "";
  /*! \brief Path to imported steam applications prefix directory. */
  QString steam_prefix_path_ = "";
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;
  /*! \brief Contains deployers which will be created upon adding a new application. */
  std::vector<EditDeployerInfo> deployers_;
  /*! \brief Contains Json objects representing imported auto tags. */
  std::vector<Json::Value> auto_tags_;
  /*! \brief Whether or not this application is running as a flatpak. */
  bool is_flatpak_;
  /*! \brief Reusable dialog for importing data from installed Steam apps. */
  std::unique_ptr<ImportFromSteamDialog> import_from_steam_dialog_;

  /*!
   * \brief Set the enabled state of this dialogs OK button.
   * \param state
   */
  void enableOkButton(bool state);
  /*! \brief Checks whether the currently entered path exists. */
  bool pathIsValid();
  /*!
   * \brief Checks whether the currently entered icon path refers to a valid icon file.
   * \param Path to an icon. If this checked instead of ui->icon_field if this is not empty.
   */
  bool iconIsValid(const QString& path = "");
  /*!
   * \brief Initializes default settings for deployers and auto tags from a file named "app_id_.json".
   * If no such file exists, creates generic deployers targeting installation directory and prefix.
   */
  void initConfigForApp();
  /*! \brief
   *  Initializes deployers targeting the currently selected steam app's installation and,
   *  if present, it's prefix directory.
   */
  void initDefaultAppConfig();

public:
  /*!
   * \brief Initializes this dialog to allow editing of an existing
   * \ref ModdedApplication "application".
   * \param name Current name of the edited \ref ModdedApplication "application".
   * \param app_version Current app app_version.
   * \param path Current staging directory path of the edited
   * \ref ModdedApplication "application".
   * \param command Current command to run the edited \ref ModdedApplication "application".
   * \param app_id Id of the edited \ref ModdedApplication "application".
   * \param steam_app_id Steam app id. Or -1 if not a Steam app.
   */
  void setEditMode(const QString& name,
                   const QString& app_version,
                   const QString& path,
                   const QString& command,
                   const QString& icon_path,
                   int app_id,
                   long steam_app_id);
  /*!
   *  \brief Initializes this dialog to allow creating a new
   *  \ref ModdedApplication "application".
   */
  void setAddMode();

private slots:
  /*! \brief Shows a file dialog for the staging directory path. */
  void on_file_picker_button_clicked();
  /*! \brief Only enable the OK button if a name has been entered. */
  void on_name_field_textChanged(const QString& text);
  /*! \brief Only enable the OK button if a valid staging directory path has been entered. */
  void on_path_field_textChanged(const QString& text);
  /*! \brief Closes the dialog and emits a signal for completion. */
  void on_buttonBox_accepted();
  /*! \brief Opens a dialog to import currently installed steam app. */
  void on_import_button_clicked();
  /*!
   * \brief Called when the import steam application dialog has been completed.
   * \param name Name of the imported application.
   * \param app_id Steam app_id of the imported application.
   * \param install_dir Name of the directory under steamapps which contains the
   * new applications files.
   * \param prefix_path Path to the applications Proton prefix, or empty if none exists.
   * \param icon_path Path to the applications icon.
   */
  void onApplicationImported(QString name,
                             QString app_id,
                             QString install_dir,
                             QString prefix_path,
                             QString icon_path);
  /*!
   * \brief Updates the staging directory path to given path.
   * \param path The new path.
   */
  void onFileDialogAccepted(const QString& path);
  /*! \brief Called when icon path picker button is clicked. */
  void on_icon_picker_button_clicked();
  /*!
   * \brief Updates the icon path to the given path if the given path refers to a valid icon.
   * \param path The new path.
   */
  void onIconPathDialogComplete(const QString& path);

signals:
  /*!
   * \brief Signals completion of the dialog in add mode.
   * \param info Contains all data entered in the dialog.
   */
  void applicationAdded(EditApplicationInfo edit_app_info);
  /*!
   * \brief Signals completion of the dialog in edit mode.
   * \param info Contains all data entered in the dialog.
   * \param app_id Id of the edited \ref ModdedApplication "application".
   */
  void applicationEdited(EditApplicationInfo edit_app_info, int app_id);
};
