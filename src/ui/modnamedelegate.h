/*!
 * \file modnamedelegate.h
 * \brief Header for the ModNameDelegate class.
 */

#pragma once

#include "modlistproxymodel.h"
#include "tablecelldelegate.h"
#include "ui/modlistview.h"
#include <QStyledItemDelegate>
#include <QTableView>


/*!
 * \brief Provides a line edit to change mod names.
 */
class ModNameDelegate : public TableCellDelegate
{
  Q_OBJECT
public:
  /*!
   * \brief Constructor.
   * \param parent Parent of this object.
   * \param proxy Proxy model used, or nullptr if non is used.
   */
  explicit ModNameDelegate(ModListProxyModel* proxy, QObject* parent);

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
   * \brief Emits \ref modNameChanged with the new mod name.
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
  void modNameChanged(int mod_id, QString name) const;
};
