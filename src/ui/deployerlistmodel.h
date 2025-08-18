/*!
 * \file deployerlistmodel.h
 * \brief Header for the DeployerListModel class.
 */

#pragma once

#include "../core/deployerinfo.h"
#include <QAbstractItemModel>
#include <QColor>

/*!
 * \brief Manages and provides access to the data displayed in the deployer list.
 */
class DeployerListModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  /*!
   * \brief Constructor.
   * \param parent Parent for this model.
   */
  explicit DeployerListModel(QObject* parent = nullptr);

  /*! \brief Index of the mod name column. */
  static constexpr int name_col = 0;
  /*! \brief Index of the mod id column. */
  static constexpr int id_col = 1;
  /*! \brief Index of the tags column. */
  static constexpr int tags_col = 2;

  /*! \brief Role representing the activation status of a mod. */
  static constexpr int mod_status_role = 300;
  /*! \brief Role representing all tags added to a mod. */
  static constexpr int mod_tags_role = 301;
  /*! \brief Rile representing whether ids are references to source mods. */
  static constexpr int ids_are_source_references_role = 302;
  /*! \brief Role representing the name of the source mod. */
  static constexpr int source_mod_name_role = 303;
  /*! \brief Role representing a list of valid mod actions. */
  static constexpr int valid_mod_actions_role = 304;

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
   *
   * Returns data depending of the given role and index. Qt standard roles are
   * used to provide data displayed in views. Custom roles defined in this file
   * and in the ModListModel header provide access to the raw data.
   * \param index Holds row and column for which to return data.
   * \param role Describes type of data to return.
   * \return The requested data.
   */
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  /*!
   * \brief Updates all data stored in this model with the given data.
   * \param mods Data for all mods managed by this model.
   */
  void setDeployerInfo(const DeployerInfo& info);
  /*!
   * \brief Only for ReverseDeployers: Whether or not profiles use separate directories.
   * \return True if separate directories are used.
   */
  bool hasSeparateDirs() const;
  /*!
   * \brief Only for ReverseDeployers: Whether or not the deployer's ignore list contains files.
   * \return True if at least one file is ignored.
   */
  bool hasIgnoredFiles() const;
  /*!
   * \brief Returns whether sorting mods is allowed affect overwrite behavior.
   *
   * If this is set to false, sorting will always be safe and only affect how mods are displayed.
   * \return The safe sorting state.
   */
  bool usesUnsafeSorting() const;

  QModelIndex index(int row, int column, const QModelIndex &parent) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  bool hasChildren(const QModelIndex &parent) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;
  void addSeparator();
  Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
  /*! \brief Contains all mods managed by this model. */
  DeployerInfo deployer_info_;
  /*! \brief Maps mod ids to the color used to display their names. */
  std::map<int, QBrush> text_colors_;
  QStringList collectTags(std::shared_ptr<TreeItem<DeployerEntry>> item) const;
};

Q_DECLARE_METATYPE(DeployerEntry *);

