/*!
 * \file backupnamedelegate.h
 * \brief Header for the BackupNameDelegate class.
 */

#pragma once

#include "ui/tablecelldelegate.h"
#include <QStyledItemDelegate>


/*!
 * \brief Provides a line edit to change backup target names.
 */
class BackupNameDelegate : public TableCellDelegate
{
  Q_OBJECT
public:
  /*!
   * \brief Constructor.
   * \param proxy Proxy model used to sort or filter the underlying model.
   * \param parent Parent view of this delegate.
   */
  explicit BackupNameDelegate(ModListProxyModel* proxy, QObject* parent);

  /*!
   * \brief Creates a QLineEdit object at the given index in the given view.
   * \param parent View for which to display the line edit.
   * \param option Style options.
   * \param index Target index for the line edit.
   * \return The new QLineEdit.
   */
  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;
  /*!
   * \brief Sets the line edits data to the name of the mod in the given row.
   * \param editor QLineEdit for which to set data.
   * \param index Index of the mod.
   */
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  /*!
   * \brief Emits \ref backupTargetNameChanged with the new mod name.
   * \param editor Line edit used to change the name.
   * \param model Ignored.
   * \param index Index for the edited name.
   */
  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override;
  /*!
   * \brief Updates the given line edits geometry.
   * \param editor Target editor.
   * \param option Style options.
   * \param index Index for the editor.
   */
  void updateEditorGeometry(QWidget* editor,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override;

signals:
  /*!
   * \brief Signals that a mod name has been changed by the user.
   * \param mod_id Target mod id.
   * \param name New name.
   */
  void backupTargetNameChanged(int target_id, QString name) const;
};
