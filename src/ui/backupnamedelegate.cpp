#include "backupnamedelegate.h"
#include "backuplistmodel.h"
#include <QLineEdit>


BackupNameDelegate::BackupNameDelegate(ModListProxyModel* proxy, QObject* parent) :
  TableCellDelegate(proxy, parent)
{}

QWidget* BackupNameDelegate::createEditor(QWidget* parent,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
  return new QLineEdit(parent);
}

void BackupNameDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  auto line_edit = static_cast<QLineEdit*>(editor);
  if(line_edit->text().isEmpty())
    line_edit->setText(index.data(BackupListModel::target_name_role).toString());
}

void BackupNameDelegate::setModelData(QWidget* editor,
                                      QAbstractItemModel* model,
                                      const QModelIndex& index) const
{
  emit backupTargetNameChanged(index.row(), static_cast<QLineEdit*>(editor)->text());
}

void BackupNameDelegate::updateEditorGeometry(QWidget* editor,
                                              const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const
{
  editor->setGeometry(option.rect);
}
