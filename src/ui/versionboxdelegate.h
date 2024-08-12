/*!
 * \file versionboxdelegate.h
 * \brief Header for the VersionBoxDelegate class.
 */

#pragma once

#include "modlistproxymodel.h"
#include "ui/modlistview.h"
#include <QStyledItemDelegate>
#include <QTableView>


/*!
 * \brief Provides either a QLineEdit or a QComboBox to edit a mods version.
 */
class VersionBoxDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  /*!
   * \brief Constructor.
   * \param proxy Proxy model used to sort or filter the underlying model.
   * \param parent Parent view of this delegate.
   */
  explicit VersionBoxDelegate(ModListProxyModel* proxy, QObject* parent);

  /*!
   * \brief Creates either a QLineEdit or a QComboBox depending on
   * whether or not the mod belongs to a group.
   * \param parent Parent view.
   * \param option Style options.
   * \param index Index at which to create the editor.
   * \return
   */
  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;
  /*!
   * \brief Initializes either the line edit with the mods version or the combo box
   * with the versions of all mods belonging to the same group.
   * \param editor Target editor.
   * \param index Index for the editor.
   */
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  /*!
   * \brief Emits \ref modVersionChanged if the mod version has been edited or
   * \ref activeGroupMemberChanged if the editor is a combo box and its index has been changed.
   * \param editor Editor used to change the data.
   * \param model Ignored.
   * \param index Index for the edited mod version.
   */
  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override;
  /*!
   * \brief Updates the given editors geometry.
   * \param editor Target editor.
   * \param option Style options.
   * \param index Index for the editor.
   */
  void updateEditorGeometry(QWidget* editor,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override;
  /*!
   * \brief If the target index references a mod in a group: Paints an editable combo box
   * at the given index into the given view.
   * \param painter Painter used to draw.
   * \param option Style options.
   * \param view_index The target views index.
   */
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& view_index) const override;
  /*!
   * \brief Sets if this is used to handle the backup list.
   * \param is_backup True for backup, else false.
   */
  void setIsBackupDelegate(bool is_backup);

private:
  /*! \brief Proxy model used to sort or filter the underlying model. */
  ModListProxyModel* proxy_model_ = nullptr;
  /*! \brief Indicates if this is used to manage the backup list. */
  bool is_backup_delegate_ = false;
  /*! \brief Convenience pointer to parent view. Points to the same address as this->parent. */
  ModListView* parent_view_;

signals:
  /*!
   * \brief Signals that the active member of a group has changed.
   * \param group Target group.
   * \param new_id New active member.
   */
  void activeGroupMemberChanged(int group, int new_id) const;
  /*!
   * \brief Signals that the version string of a mod has changed.
   * \param mod_id Target mod.
   * \param version New version string.
   */
  void modVersionChanged(int mod_id, QString version) const;
  /*!
   * \brief Signals that the active backup for the current backup target has changed.
   * \param target Current target.
   * \param backup New active backup.
   */
  void activeBackupChanged(int target, int backup) const;
  /*!
   * \brief Signals that the name of a backup has been edited by the user.
   * \param target Target to which the backup belongs.
   * \param backup The edited backup.
   * \param name The new name.
   */
  void backupNameEdited(int target, int backup, QString name) const;
};
