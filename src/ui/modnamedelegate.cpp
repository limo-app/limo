#include "modnamedelegate.h"
#include "modlistmodel.h"
#include <QApplication>
#include <QHeaderView>
#include <QLineEdit>
#include <QPainter>


ModNameDelegate::ModNameDelegate(ModListProxyModel* proxy, QObject* parent) :
  TableCellDelegate(proxy, parent)
{}

QWidget* ModNameDelegate::createEditor(QWidget* parent,
                                       const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const
{
  auto line_edit = new QLineEdit(parent);
  line_edit->setText(index.data(ModListModel::mod_name_role).toString());
  return line_edit;
}

void ModNameDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  //  auto line_edit = static_cast<QLineEdit*>(editor);
  //  if(line_edit->text().isEmpty())
  //    line_edit->setText(index.data(ModListModel::mod_name_role).toString());
}

void ModNameDelegate::setModelData(QWidget* editor,
                                   QAbstractItemModel* model,
                                   const QModelIndex& index) const
{
  emit modNameChanged(index.data(ModListModel::mod_id_role).toInt(),
                      static_cast<QLineEdit*>(editor)->text());
}

void ModNameDelegate::updateEditorGeometry(QWidget* editor,
                                           const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const
{
  editor->setGeometry(option.rect);
}
