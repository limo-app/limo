/*!
 * \file nexusmoddialog.h
 * \brief Header for the NexusModDialog class.
 */

#pragma once

#include "src/core/nexus/api.h"
#include "tabletoolbutton.h"
#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>


namespace Ui
{
class NexusModDialog;
}

/*!
 * \brief Dialog used to display the descrition page, the changelogs and all available files
 * for a mod on NexusMods.
 */
class NexusModDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit NexusModDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~NexusModDialog();

  /*!
   * \brief Initializes the dialog with the data for the given Page.
   * \param app_id ModdedApplication to which the mod, whose page is being shown, belongs.
   * \param mod_id Limo mod id for the mod to which the page belongs.
   * \param page Contains all data necessary for the dialog.
   */
  void setupDialog(int app_id, int mod_id, const nexus::Page& page);

private:
  /*! \brief Contains auto-generated UI elements. */
  Ui::NexusModDialog* ui;
  /*! \brief ModdedApplication to which the mod, whose page is being shown, belongs. */
  int app_id_;
  /*! \brief Limo mod id for the mod to which the page belongs. */
  int mod_id_;
  /*! \brief Contains all data necessary for the dialog. */
  nexus::Page page_;

  /*!
   * \brief Converts the given BBCode formatted string to a string formatted with HTML tags.
   * \param bbcode String to convert.
   * \return The converted string.
   */
  QString bbcodeToHtml(const QString& bbcode);

private slots:
  /*!
   * \brief Sends a download request for the given file.
   * \param file_id Id of the NexusMods file to download.
   * \param file_id_copy Id of the NexusMods file to download.
   */
  void onDownloadClicked(int file_id, int file_id_copy);

signals:
  /*!
   * \brief Sends a download request for the given file.
   * \param app_id ModdedApplication to which the mod, whose page is being shown, belongs.
   * \param mod_id Limo mod id for the mod to which the page belongs.
   * \param file_id NexusMods file id used for the download.
   * \param mod_url URL of the mod page on NexusMods.
   */
  void modDownloadRequested(int app_id, int mod_id, int file_id, QString mod_url);
};
