/*!
 * \file edittoolwidget.h
 * \brief Header for the EditToolWidget class.
 */

#pragma once

#include "core/tool.h"
#include "importfromsteamdialog.h"
#include "validatinglineedit.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>


/*!
 * \brief A QWidget that allows creating and editing \ref Tool "tools".
 */
class EditToolWidget : public QWidget
{
  Q_OBJECT
public:

  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QWidget.
   */
  explicit EditToolWidget(QWidget* parent = nullptr);
  /*!
   * \brief Constructs a Tool from the input data.
   * \return The constructed Tool.
   */
  Tool getTool();
  /*!
   * \brief Returns whether or not the current input is valid.
   * \return True if valid.
   */
  bool hasValidInput() const;
  /*! \brief Initializes the widget with empty fields. */
  void init();
  /*!
   * \brief Initializes the widget with data from given Tool.
   * \param tool Source Tool.
   */
  void init(const Tool& tool);

private:
  /*! \brief Index representing guided mode in the mode box. */
  static constexpr int MODE_GUIDED_INDEX = 0;
  /*! \brief Index representing manual mode in the mode box. */
  static constexpr int MODE_MANUAL_INDEX = 1;
  /*! \brief Index representing native runtime in the runtime box. */
  static constexpr int RUNTIME_NATIVE_INDEX = 0;
  /*! \brief Index representing wine runtime in the runtime box. */
  static constexpr int RUNTIME_WINE_INDEX = 1;
  /*! \brief Index representing protontricks runtime in the runtime box. */
  static constexpr int RUNTIME_PROTONTRICKS_INDEX = 2;
  /*! \brief Index representing steam runtime in the runtime box. */
  static constexpr int RUNTIME_STEAM_INDEX = 3;
  /*! \brief Index representing the action column in the environment table. */
  static constexpr int ENVIRONMENT_ACTION_COL = 0;
  /*! \brief Index representing the variable column in the environment table. */
  static constexpr int ENVIRONMENT_VARIABLE_COL = 1;
  /*! \brief Index representing the value column in the environment table. */
  static constexpr int ENVIRONMENT_VALUE_COL = 2;
  /*! \brief Index representing native version in the runtime version box. */
  static constexpr int VERSION_NATIVE_INDEX = 0;
  /*! \brief Index representing flatpak version in the runtime version box. */
  static constexpr int VERSION_FLATPAK_INDEX = 1;

  /*! \brief Label used for the mode. */
  QLabel* mode_label_;
  /*! \brief Combo box used to select the mode. */
  QComboBox* mode_box_;
  /*! \brief Label used for the name. */
  QLabel* name_label_;
  /*! \brief Input field for the name. */
  ValidatingLineEdit* name_field_;
  /*! \brief Label used for the icon. */
  QLabel* icon_label_;
  /*! \brief Input field for the icon path. */
  ValidatingLineEdit* icon_field_;
  /*! \brief Push button to open a file dialog for selecting the icon path. */
  QPushButton* icon_picker_;
  /*! \brief Label used for the executable path. */
  QLabel* executable_label_;
  /*! \brief Input field for the executable path. */
  ValidatingLineEdit* executable_field_;
  /*! \brief Push button to open a file dialog for selecting the executable path. */
  QPushButton* executable_picker_;
  /*! \brief Label used for the runtime. */
  QLabel* runtime_label_;
  /*! \brief Combo box used to select the runtime. */
  QComboBox* runtime_box_;
  /*! \brief Label used for the runtime version. */
  QLabel* runtime_version_label_;
  /*! \brief Combo box used to select the runtime version. */
  QComboBox* runtime_version_box_;
  /*! \brief Label used for the prefix path. */
  QLabel* prefix_label_;
  /*! \brief Input field for the prefix path. */
  ValidatingLineEdit* prefix_field_;
  /*! \brief Push button to open a file dialog for selecting the prefix path. */
  QPushButton* prefix_picker_;
  /*! \brief Label used for the app id. */
  QLabel* app_id_label_;
  /*! \brief Input field for the app id. */
  ValidatingLineEdit* app_id_field_;
  /*! \brief Push button to open the ImportFromSteamDialog. */
  QPushButton* app_id_import_button_;
  /*! \brief Label used for the working directory. */
  QLabel* working_directory_label_;
  /*! \brief Input field for the working directory. */
  ValidatingLineEdit* working_directory_field_;
  /*! \brief Push button to open a file dialog for selecting the working directory. */
  QPushButton* working_directory_picker_;
  /*! \brief Label used for the environment table. */
  QLabel* environment_label_;
  /*! \brief Table used to display environment variables. */
  QTableWidget* environment_table_;
  /*! \brief Label used for the arguments. */
  QLabel* arguments_label_;
  /*! \brief Input field for the arguments. */
  QLineEdit* arguments_field_;
  /*! \brief Label used for the protontricks arguments. */
  QLabel* protontricks_arguments_label_;
  /*! \brief Input field for the protontricks arguments. */
  QLineEdit* protontricks_arguments_field_;
  /*! \brief Label used for the command. */
  QLabel* command_label_;
  /*! \brief Input field for the command. */
  ValidatingLineEdit* command_field_;
  /*! \brief Dialog used to import app ids and icon paths from steam. */
  ImportFromSteamDialog* import_dialog_;

  /*! \brief Contains pairs of environment variables and their assigned values. */
  std::vector<std::pair<QString, QString>> environment_variables;
  /*! \brief If true: Current input is valid. */
  bool has_valid_input_ = false;

  /*! \brief Updates the visibility of child widgets according to the selected mode and runtime. */
  void updateChildrenVisibility();
  /*!
   * \brief Shows a file dialog and writes the selected path to the given line edit.
   * \param target_field Line edit to which to write the path.
   * \param title Title of the file dialog.
   * \param directories_only If true: File dialog will only show directories.
   */
  void runFileDialog(QLineEdit* target_field, const QString& title, bool directories_only);
  /*! \brief Updates the environment table with the data in environment_variables. */
  void updateEnvironmentTable();

signals:
  /*!
   * \brief Signals the the validity of current input data has changed.
   * \param is_valid The validity status.
   */
  void inputValidityChanged(bool is_valid);

private slots:
  /*!
   * \brief Updates children visibility according to selected mode.
   * \param index The new mode.
   */
  void modeBoxIndexChanged(int index);
  /*!
   * \brief Updates children visibility according to selected runtime.
   * \param index The new runtime.
   */
  void runtimeBoxIndexChanged(int index);
  /*! \brief Opens a file dialog to select an executable path. */
  void executablePickerClicked();
  /*! \brief Opens a file dialog to select a prefix path. */
  void prefixPickerClicked();
  /*! \brief Opens a file dialog to select a working directory. */
  void workingDirPickerClicked();
  /*! \brief Opens a file dialog to select an icon path. */
  void iconPickerClicked();
  /*!
   * \brief Removes the environment variable in the given row in the environment table from
   * environment_variables.
   * \param row Row to remove.
   * \param col Ignored.
   */
  void environmentVariableRemoved(int row, int col);
  /*! \brief Adds a new empty environment variable. */
  void environmentVariableAdded();
  /*!
   * \brief Updates the environment variable in the given row in the environment table in
   * environment_variables.
   * \param row Edited row.
   * \param col Edited column.
   */
  void environmentTableCellChanged(int row, int col);
  /*!
   * \brief Updates input validity status.
   * \param new_text Ignored.
   */
  void textFieldEdited(QString new_text);
  /*! \brief Opens an ImportFromSteamDialog. */
  void importButtonClicked();
  /*!
   * \brief Updates the app id field and if runtime is steam also the icon path field
   * with given data.
   * \param name Ignored.
   * \param app_id Imported app id.
   * \param install_dir Ignored.
   * \param prefix_path Ignored.
   * \param icon_path Imported icon path.
   */
  void steamAppImported(QString name,
                        QString app_id,
                        QString install_dir,
                        QString prefix_path,
                        QString icon_path);
};
