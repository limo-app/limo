/*!
 * \file conflictsmodel.h
 * \brief Header for the ConflictsModel class.
 */

#pragma once

#include "../core/conflictinfo.h"
#include <QAbstractTableModel>


/*!
 * \brief Manages and provides access to the data displayed in the file conflicts window.
 */
class ConflictsModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  /*!
   * \brief Constructor.
   * \param parent Parent for this model.
   */
  explicit ConflictsModel(QObject* parent = nullptr);

  /*! \brief Index of the file path column. */
  static constexpr int file_col = 0;
  /*! \brief Index of the winning mod name column. */
  static constexpr int name_col = 1;
  /*! \brief Index of the winning mod id column. */
  static constexpr int id_col = 2;

  /*!
   * \brief Returns the horizontal header section names and vertical header section indices.
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
   * \param index Holds row and column for which to return data.
   * \param role Describes type of data to return.
   * \return The requested data.
   */
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  /*!
   * \brief Updates the data in this model with the new conflicts.
   * \param newConflicts Contains file, winner name and winner id for every conflict.
   * \param base_id Id of the mod for which the conflicts are displayed.
   */
  void setConflicts(const std::vector<ConflictInfo>& newConflicts, int base_id);

private:
  /*! \brief For every conflict: File, winner name and winner id */
  std::vector<ConflictInfo> conflicts_;
  /*! \brief Id of the mod for which the conflicts are displayed. */
  int base_id_;
};
