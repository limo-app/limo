/*!
 * \file modlistproxymodel.h
 * \brief Header for the ModListProxyModel class.
 */

#pragma once

#include <QLabel>
#include <QSortFilterProxyModel>


/*!
 * \brief Used to sort or filter the mod list.
 */
class ModListProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  /*! \brief Describes different filter types. */
  enum FilterMode
  {
    /*! \brief Show only mods in groups. */
    filter_groups = 1,
    /*! \brief Show only mods without groups. */
    filter_no_groups = 2,
    /*! \brief Show only mods not managed by any deployer. */
    filter_inactive = 4,
    /*! \brief Show only mods managed by at least one deployer. */
    filter_active = 8,
    /*! \brief Show only mods with the given tags. */
    filter_tags = 16,
    /*! \brief Show only mods with available updates from their remote source. */
    filter_updates = 32,
    /*! \brief Show only mods without available updates from their remote source. */
    filter_no_updates = 64
  };

  /*!
   * \brief Constructor.
   * \param row_count_label Used to display to total number of rows.
   * \param parent Parent of this object.
   */
  explicit ModListProxyModel(QLabel* row_count_label, QObject* parent = nullptr);

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
  /*! \brief Updates the row count label with the current number of rows. */
  void updateRowCountLabel();
  /*!
   * \brief Returns true if the source model is editable;
   * \return True if editable.
   */
  bool isEditable() const;
  /*!
   * \brief Returns the currently set tag filters.
   * \return Pairs of tag names and bools indicating if they should be kept or removed while filtering.
   */
  std::vector<std::pair<QString, bool>> getTagFilters() const;
  /*!
   * \brief Sets the string to use for filtering.
   * \param filter_string The new filter string.
   */
  void setFilterString(const QString& filter_string);

protected:
  /*!
   * \brief Compares two entries for sorting operations. Uses the base class implementation for
   * all rows except for size_row, since the size strings cannot be alphabetically compared.
   * \param left First index.
   * \param right Second index.
   * \return True if left < right.
   */
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
  /*! \brief Contains the sum of all currently active filter modes. */
  int filter_mode_ = 0;
  /*!
   * \brief Contains every tag to be filtered as well as a bool indicating whether
   *  mods with that tag show be shown or not shown.
   */
  std::vector<std::pair<QString, bool>> tag_filters_;
  /*! \brief Used to display to total number of rows. */
  QLabel* row_count_label_;
  /*! \brief String used to filter rows. */
  QString filter_string_;
  /*! \brief Regex used to check if filter_string_ is used to filter for ids. */
  const QRegularExpression id_regex_;
  /*! \brief True if the current filter string is an integer. */
  bool filter_string_is_int_ = false;
  /*! \brief If the current filter string is an integer, this contains that integer. */
  QString filter_string_id_;
  /*! \brief True if filter string matches id_regex_. */
  bool filter_string_targets_id_ = false;
};
