#include "versionboxdelegate.h"
#include "backuplistmodel.h"
#include "modlistmodel.h"
#include "modlistview.h"
#include "qmodelindexutils.h"
#include "ui/colors.h"
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QHeaderView>
#include <QLineEdit>
#include <QPainter>

VersionBoxDelegate::VersionBoxDelegate(ModListProxyModel* proxy, QObject* parent) :
  QStyledItemDelegate{ parent }, proxy_model_(proxy),
  parent_view_(static_cast<ModListView*>(parent))
{}

QWidget* VersionBoxDelegate::createEditor(QWidget* parent,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
  const auto versions = index.data(Qt::UserRole).value<QStringList>();
  if(versions.size() > 1)
  {
    auto box = new QComboBox(parent);
    box->addItems(versions);
    box->setCurrentIndex(index.data(ModListModel::active_index_role).value<int>());
    box->adjustSize();
    box->setGeometry(option.rect);
    const int cursor_x_pos = parent_view_->mapFromGlobal(QCursor::pos()).x();
    if(option.rect.right() - 18 < cursor_x_pos)
      box->showPopup();
    else
      box->setEditable(true);
    return box;
  }
  else
  {
    auto line_edit = new QLineEdit(parent);
    line_edit->setText(versions[0]);
    return line_edit;
  }
}

void VersionBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {}

void VersionBoxDelegate::setModelData(QWidget* editor,
                                      QAbstractItemModel* model,
                                      const QModelIndex& index) const
{
  const auto versions = index.data(ModListModel::version_list_role).value<QStringList>();
  if(!is_backup_delegate_)
  {
    if(versions.size() > 1)
    {
      auto box = static_cast<QComboBox*>(editor);
      int box_index = box->currentIndex();
      int cur_active = index.data(ModListModel::active_index_role).value<int>();
      if(box_index != cur_active &&
         box_index < index.data(Qt::UserRole).value<QStringList>().size())
      {
        const int group = index.data(ModListModel::mod_group_role).value<int>();
        const auto ids = index.data(ModListModel::group_members_role).value<std::vector<int>>();
        emit activeGroupMemberChanged(group, ids[box_index]);
      }
      else if(box->currentText() != versions[cur_active])
        emit modVersionChanged(index.data(ModListModel::mod_id_role).value<int>(),
                               box->currentText());
    }
    else
    {
      const QString text = static_cast<QLineEdit*>(editor)->text();
      emit modVersionChanged(index.data(ModListModel::mod_id_role).value<int>(), text);
    }
  }
  else
  {
    if(versions.size() > 1)
    {
      auto box = static_cast<QComboBox*>(editor);
      // for some reason, if editing was completed by pressing enter,
      // one item is temporarily appended to the combo box and the boxes current index is
      // set to that new entry
      int box_index = box->currentIndex();
      int cur_active = index.data(ModListModel::active_index_role).value<int>();
      if(box_index != cur_active &&
         box_index < index.data(BackupListModel::num_backups_role).value<int>())
      {
        emit activeBackupChanged(index.row(), box_index);
      }
      else if(box->currentText() != versions[cur_active])
        emit backupNameEdited(index.row(), cur_active, box->currentText());
    }
    else
    {
      const QString text = static_cast<QLineEdit*>(editor)->text();
      emit backupNameEdited(index.row(), 0, text);
    }
  }
}

void VersionBoxDelegate::updateEditorGeometry(QWidget* editor,
                                              const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const
{
  editor->setGeometry(option.rect);
}

void VersionBoxDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& view_index) const
{
  auto model_index = proxy_model_ == nullptr ? view_index : proxy_model_->mapToSource(view_index);
  QStyleOptionComboBox box;
  const QRect rect = option.rect;
  box.rect = rect;
  box.currentText = model_index.data().toString();
  box.editable = true;
  box.state |= QStyle::State_Enabled;
  const auto mouse_row = parent_view_->getHoverRow();
  if(!is_backup_delegate_ &&
     !parent_view_->selectionModel()->rowIntersectsSelection(view_index.row()))
  {
    auto fg_brush = view_index.data(Qt::ForegroundRole);
    if(!fg_brush.isNull())
    {
      box.palette.setBrush(QPalette::WindowText, fg_brush.value<QBrush>());
      box.palette.setBrush(QPalette::ButtonText, fg_brush.value<QBrush>());
    }
  }
  if(parent_view_->selectionModel()->rowIntersectsSelection(view_index.row()))
  {
    box.palette.setBrush(
      QPalette::Base,
      option.palette.color(parent_view_->hasFocus() ? QPalette::Active : QPalette::Inactive,
                           QPalette::Highlight));
  }
  else if(sameRow(mouse_row, view_index))
  {
    const float color_ratio = 0.8;
    auto hl_color = option.palette.color(QPalette::Highlight);
    auto bg_color =
      option.palette.color(view_index.row() % 2 ? QPalette::AlternateBase : QPalette::Base);
    auto mix_color = hl_color;
    mix_color.setRed(hl_color.red() * (1 - color_ratio) + bg_color.red() * color_ratio);
    mix_color.setGreen(hl_color.green() * (1 - color_ratio) + bg_color.green() * color_ratio);
    mix_color.setBlue(hl_color.blue() * (1 - color_ratio) + bg_color.blue() * color_ratio);
    box.palette.setBrush(QPalette::Base, QBrush(mix_color));
  }
  else if(view_index.row() % 2)
    box.palette.setBrush(QPalette::Base, option.palette.alternateBase());
  if(!is_backup_delegate_ && model_index.data(ModListModel::mod_group_role).value<int>() >= 0 ||
     is_backup_delegate_ && model_index.data(BackupListModel::num_backups_role).value<size_t>() > 1)
    QApplication::style()->drawComplexControl(QStyle::CC_ComboBox, &box, painter);
  else
  {
    QPixmap map(rect.width(), rect.height());
    map.fill(box.palette.color(QPalette::Base));
    painter->drawPixmap(rect, map);
  }
  box.editable = false;
  QApplication::style()->drawControl(QStyle::CE_ComboBoxLabel, &box, painter);
  if(!parent_view_->selectionModel()->rowIntersectsSelection(view_index.row()) &&
     parent_view_->selectionModel()->currentIndex().row() == view_index.row())
  {
    QStyleOptionFocusRect indicator;
    indicator.rect = option.rect;
    QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &indicator, painter);
  }
}

void VersionBoxDelegate::setIsBackupDelegate(bool is_backup)
{
  is_backup_delegate_ = is_backup;
}
