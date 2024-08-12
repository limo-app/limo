#pragma once

#include "modlistview.h"


class BackupListView : public ModListView
{
  Q_OBJECT
public:
  /*!
   * \brief Simply calls ModListView's constructor with parent as argument.
   * \param parent The parent widget for this widget.
   */
  explicit BackupListView(QWidget* parent = nullptr);

  /*!
   * \brief Gets called when the mouse has been pressed while in this widget.
   *
   * Highlights the currently selected row and forwards edit events.
   * \param event The source event.
   */
  void mousePressEvent(QMouseEvent* event) override;
  /*!
   * \brief Gets called when the mouse has been released while in this widget.
   *
   * Forwards event for pressed buttons in the view.
   * \param event The source event.
   */
  void mouseReleaseEvent(QMouseEvent* event) override;
  /*!
   * \brief Gets called when a double click has been performed in this widget.
   *
   * Calls the respective editor for mod name and mod version name.
   * \param event The source event.
   */
  void mouseDoubleClickEvent(QMouseEvent* event) override;

signals:
  /*!
   * \brief Signals that a backup target has been removed.
   * \param target_id Id of the backup target which is to be removed.
   * \param name Name of the target.
   */
  void backupTargetRemoved(int target_id, QString name);
  /*! \brief Signals that a backup target has been added. */
  void addBackupTargetClicked();
};
