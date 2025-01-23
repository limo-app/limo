/*!
 * \file changelogdialog.h
 * \brief Header for the ChangelogDialog class
 */

#pragma once

#include <QDialog>
#include "core/versionchangelog.h"


namespace Ui
{
class ChangelogDialog;
}

/*!
 * \brief Dialog used to display changelogs for Limo versions.
 */
class ChangelogDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Reads all changes from the changelog file.
   * \param is_flatpak If true: This app is running inside of a flatpak sandbox.
   * \param parent Parent widget.
   */
  explicit ChangelogDialog(bool is_flatpak, QWidget* parent = nullptr);
  /*! \brief Deletes the UI. */
  ~ChangelogDialog();
  /*!
   * \brief Checks if changelogs could be found.
   * \return True if found.
   */
  bool hasChanges() const;

private slots:
  /*!
   * \brief Updates the changelog text to show all changelogs since the selected version.
   * \param index Selected version index.
   */
  void on_version_box_currentIndexChanged(int index);

private:
  /*! \brief Contains auto.generated UI elements. */
  Ui::ChangelogDialog* ui;
  /*! \brief If true: This app is running inside of a flatpak sandbox. */
  bool is_flatpak_;
  /*! \brief For every version: The changelog for that version. */
  std::vector<VersionChangelog> versions_;
  /*! \brief Indicates that changes exist. */
  bool has_changes_ = false;
};
