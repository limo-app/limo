/*!
 * \file backuplistmodel.h
 * \brief Header for the BackupListModel class.
 */

#pragma once

#include "core/backuptarget.h"
#include "ui/modlistmodel.h"
#include <QAbstractTableModel>

/*!
 * \brief Manages and provides access to the data displayed in the backup list.
 */
class BackupListModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  /*!
   * \brief Constructor.
   * \param parent The QTableView used to display the data in this mode.
   */
  explicit BackupListModel(QObject* parent = nullptr);

  /*! \brief Index of the action column. */
  static constexpr int action_col = 0;
  /*! \brief Index of the target column. */
  static constexpr int target_col = 1;
  /*! \brief Index of the backup column. */
  static constexpr int backup_col = 2;
  /*! \brief Index of the path column. */
  static constexpr int path_col = 3;
  /*! \brief Role representing the list of backups for a target. */
  static constexpr int backup_list_role = 256;
  /*! \brief Role representing the id of the currently active backup for a target. */
  static constexpr int active_index_role = ModListModel::active_index_role;
  /*! \brief Role representing the number of backups for a target. */
  static constexpr int num_backups_role = active_index_role + 1;
  /*! \brief Role representing the total number of target. */
  static constexpr int num_targets_role = num_backups_role + 1;
  /*! \brief Role representing the name of a target. */
  static constexpr int target_name_role = num_targets_role + 1;
  /*! \brief Role representing the name of the currently active backup for a target. */
  static constexpr int backup_name_role = target_name_role + 1;
  /*! \brief Role representing the path for a target. */
  static constexpr int target_path_role = backup_name_role + 1;

  /*!
   * \brief Returns the horizontal header section names.
   * \param section Target section.
   * \param orientation Header orientation.
   * \param role Data role.
   * \return Name of the section.
   */
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  /*!
   * \brief Returns the number of rows to display.
   * \param parent Parent index.
   * \return The number of rows.
   */
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  /*!
   * \brief Returns the number of columns to display.
   * \param parent Parent index.
   * \return The number of columns.
   */
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  /*!
   * \brief Used to access the data stored in this model.
   *
   * Returns data depending on the given role and index. Qt standard roles are
   * used to provide data displayed in views. Custom roles defined in this file
   * provide access to the raw data.
   * \param index Hold row and column for which to return data. Column data is ignored.
   * \param role Describes type of data to return.
   * \return The requested data.
   */
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  /*!
   * \brief Returns the flags for the given index. Adds editing flags for editable columns.
   * \param index Target index.
   * \return The flags.
   */
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  /*!
   * \brief Sets the data for all backup targets.
   * \param targets The new targets.
   */
  void setBackupTargets(const std::vector<BackupTarget>& targets);
  /*!
   * \brief Enables or disables the ability to edit this models data in a view.
   * \param is_editable Model data will be editable if this is true.
   */
  void setIsEditable(bool is_editable);
  /*!
   * \brief Checks if the model is currently editable.
   * \return True if the model is editable, else false.
   */
  bool isEditable() const;

private:
  /*! \brief Stores data for all displayed backup targets. */
  std::vector<BackupTarget> targets_;
  /*! \brief Stores whether the model can be edited in a view. */
  bool is_editable_ = true;
};
