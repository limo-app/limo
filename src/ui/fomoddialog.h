/*!
 * \file fomodialog.h
 * \brief Header for the FomodDialog class.
 */

#pragma once

#include "../core/addmodinfo.h"
#include "../core/fomod/fomodinstaller.h"
#include <QButtonGroup>
#include <QDialog>
#include <filesystem>
#include <set>


namespace Ui
{
class FomodDialog;
}

/*!
 * \brief Dialog used to interact with a FomodInstaller.
 */
class FomodDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent of this widget.
   */
  FomodDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~FomodDialog();

  /*!
   * \brief Initializes the dialog.
   * \param config_file Either a direct path to a fomod configuration file
   * or a path to a directory containing fomod/ModuleConfig.xml.
   * \param target_path Path used by the FomodInstaller to check for dependencies.
   * \param app_version App version used for fomod conditions. All version
   * checks evaluate to true if this is left empty.
   * \param info Contains necessary data to install the mod upon dialog completion.
   * \param app_id Application for which the new mod is to be installed.
   */
  void setupDialog(const std::filesystem::path& config_file,
                   const std::filesystem::path& target_path,
                   const QString& app_version,
                   const AddModInfo& info,
                   int app_id);
  /*!
   * \brief Returns pairs of source and destinations for every selected file during
   * the installation process.
   * \return The files.
   */
  std::vector<std::pair<std::filesystem::path, std::filesystem::path>> getResult() const;
  /*!
   * \brief Checks if the dialog has any steps.
   * \return True if there are steps.
   */
  bool hasSteps() const;

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::FomodDialog* ui;
  /*! \brief Used to parse the fomod file. */
  std::unique_ptr<fomod::FomodInstaller> installer_;
  /*! \brief Contains the current step in the installation process. */
  fomod::InstallStep cur_step_;
  /*! \brief Buttons used to represent plugin options. */
  QList<QButtonGroup*> button_groups_;
  /*! \brief Determines selection restrictions for button groups. */
  QList<fomod::PluginGroup::Type> group_types_;
  /*! \brief Button used to advance the dialog. */
  QPushButton* next_button_;
  /*! \brief Button used to step back in the dialog. */
  QPushButton* back_button_;
  /*! \brief Some button groups require a dummy button. This holds the ids of those groups. */
  std::set<int> none_groups_;
  /*! \brief Once the installation has been completed, this contains source and destination
   *  paths for every file selected during the installation. */
  std::vector<std::pair<std::filesystem::path, std::filesystem::path>> result_;
  /*! \brief If true: This dialog is non interactive. */
  bool has_no_steps_;
  /*! \brief Contains necessary data to install the mod upon dialog completion. */
  AddModInfo add_mod_info_;
  /*! \brief Application for which the new mod is to be installed. */
  int app_id_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

  /*!
   * \brief Creates a new FomodCheckBox or FomodRatioButton for selection of a plugin.
   * \param type Generated button type is determined based on this plugin type.
   * \param text Text of the new button.
   * \param description Description of the plugin represented by this button.
   * \param image_path Path to an image for the plugin represented by this button.
   * \return Pointer to the new button.
   */
  QAbstractButton* makeButton(fomod::PluginGroup::Type type,
                              const QString& text,
                              const QString& description,
                              const QString& image_path) const;
  /*!
   * \brief Takes one step forwards or backwards in the installation and updates the UI.
   * \param prev_step Pair of the selection and installer state during the previous step.
   * If this is empty, the installer advances, else the installer takes a step back.
   */
  void updateInstallStep(
    std::optional<std::pair<std::vector<std::vector<bool>>, fomod::InstallStep>> prev_step = {});
  /*!
   * \brief Verifies if the current selection satisfies the requirements to advance to
   * the next step.
   * \return True if valid.
   */
  bool selectionIsValid();
  /*!
   * \brief For every group, for every plugin: True if plugin is currently selected.
   * \return The selection.
   */
  std::vector<std::vector<bool>> getSelection();
  /*! \brief Updates text and enabled status of next_button_, depending on the step. */
  void updateNextButton();
  /*!
   * \brief Closes the dialog and emits a signal indicating installation has been canceled.
   * \param event The close even sent upon closing the application.
   */
  void closeEvent(QCloseEvent* event) override;

private slots:
  /*! \brief Advances the dialog by one step. */
  void onNextButtonPressed();
  /*!
   * \brief Called when a button has been pressed, updates the next button.
   * \param checked Button check state.
   */
  void onPluginSelected(bool checked);
  /*! \brief Takes a step back. */
  void onBackButtonPressed();
  /*! \brief Emits addModAborted */
  void on_buttonBox_rejected();

signals:
  /*!
   * \brief Signals dialog completion.
   * \param app_id Application for which the new mod is to be installed.
   * \param info Contains all data needed to install the mod.
   */
  void addModAccepted(int app_id, AddModInfo info);
  /*! \brief Signals mod installation has been aborted. */
  void addModAborted();
};
