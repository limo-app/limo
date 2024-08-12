/*!
 * \file deployerlistproxymodel.h
 * \brief Header for the DeployerListProxyModel class.
 */

#pragma once

#include <QLabel>
#include <QSortFilterProxyModel>
#include <unordered_set>


/*!
 * \brief Used to sort and filter the deployer list.
 */
class DeployerListProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  /*! \brief Describes different filter types. */
  enum FilterMode
  {
    /*! \brief Show only inactive mods. */
    filter_inactive = 1,
    /*! \brief Show only active mods. */
    filter_active = 2,
    /*! \brief Show only mods in the conflicts_ set. */
    filter_conflicts = 4,
    /*! \brief Show only mods with given tags. */
    filter_tags = 8
  };

  /*!
   * \brief Constructor.
   * \param row_count_label Used to display to total number of rows.
   * \param parent Parent of this object.
   */
  explicit DeployerListProxyModel(QLabel* row_count_label, QObject* parent = nullptr);

  /*!
   * \brief Maps the given index to the source models index and returns the source models data
   * at that index.
   * \param index Index to remap.
   * \param role Data role to get.
   * \return The source models data, depending on the given role.
   */
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  /*!
   * \brief Maps the given index to the source mods index and sets its data to the given new data.
   * \param index Index to remap.
   * \param value New data.
   * \param role Data role to edit.
   * \return True if the data has been changed, else false.
   */
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  /*!
   * \brief Sets the given filter mode to the given status.
   * \param mode Filter mode to set.
   * \param status If true: Enable the filter, else disable it.
   * \param invalidate_filter If true: Update filtered mods with the current settings.
   */
  void setFilterMode(FilterMode mode, bool status, bool invalidate_filter = true);
  /*!
   * \brief Enables the given filter mode.
   * \param mode Filter mode to activate.
   * \param invalidate_filter If true: Update filtered mods with the current settings.
   */
  void addFilter(FilterMode mode, bool invalidate_filter = true);
  /*!
   * \brief Disables the given filter mode.
   * \param mode Filter mode to activate.
   * \param invalidate_filter If true: Update filtered mods with the current settings.
   */
  void removeFilter(FilterMode mode, bool invalidate_filter = true);
  /*!
   * \brief Checks if the given row will be accepted by the current filter.
   * \param source_row Row to check.
   * \param source_parent Parent model containing the data filtered by this proxy.
   * \return True if the row is accepted.
   */
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
  /*!
   * \brief Resets the filter to allow all mods.
   * \param invalidate_filter If true: Update filtered mods with the current settings.
   */
  void clearFilter(bool invalidate_filter = true);
  /*!
   * \brief Getter for the current filter mode.
   * \return The filter mode. The integer is the sum of all currently active FilterModes.
   */
  int getFilterMode();
  /*!
   * \brief Sets data for the conflicts filter.
   * \param conflicts A set containing ids of all mods with conflicts.
   * Only mods in this set will be shown when the conflicts filter mode is active.
   */
  void setConflicts(const std::unordered_set<int>& conflicts);
  /*!
   * \brief Adds a tag to the tag filters.
   * \param tag Tag to be added.
   * \param include If true: Only show mods with this tag, else only show mods without.
   * \param invalidate_filter If true: Update filtered mods with the current settings.
   */
  void addTagFilter(const QString& tag, bool include, bool invalidate_filter);
  /*!
   * \brief Removes the filter for the given tag.
   * \param tag Tag to be removed.
   * \param invalidate_filter If true: Update filtered mods with the current settings.
   */
  void removeTagFilter(const QString& tag, bool invalidate_filter);
  /*!
   * \brief Generates a map used for row coloring from the given conflict groups.
   * \param groups For every conflict group: A vector of mod ids belonging to that group.
   */
  void setConflictGroups(const std::vector<std::vector<int>>& groups);
  /*! \brief Updates the row count label with the current number of rows. */
  void updateRowCountLabel();
  /*!
   * \brief Updates the filter and row coloring.
   * \param If true: Update filtered mods with the current settings.
   */
  void updateFilter(bool invalidate = true);
  /*!
   * \brief Returns the currently set tag filters.
   * \return Pairs of tag names and bools indicating if they should be kept or removed while filtering.
   */
  std::vector<std::pair<QString, bool>> getTagFilters() const;

private:
  /*! \brief Contains the sum of all currently active filter modes. */
  int filter_mode_ = 0;
  /*! \brief Contains all mod ids to be shown when the conflicts filter is active. */
  std::unordered_set<int> conflicts_;
  /*!
   * \brief Contains every tag to be filtered as well as a bool indicating whether
   *  mods with that tag show be shown or not shown.
   */
  std::vector<std::pair<QString, bool>> tag_filters_;
  /*! \brief Maps mod ids to their conflict group. */
  std::map<int, int> conflict_groups_;
  /*! \brief Id of the conflict group that contains mods without conflicts. */
  int no_conflict_group_ = 0;
  /*! \brief For every displayed row: The text color used. */
  std::vector<QBrush> row_text_colors_;
  /*! \brief Used to display to total number of rows. */
  QLabel* row_count_label_;
};
