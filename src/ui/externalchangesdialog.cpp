#include "externalchangesdialog.h"
#include "ui_externalchangesdialog.h"

ExternalChangesDialog::ExternalChangesDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::ExternalChangesDialog)
{
  ui->setupUi(this);
  ui->actionKeep_All->setIcon(QIcon::fromTheme("edit-select-all"));
  ui->actionKeep_None->setIcon(QIcon::fromTheme("edit-select-none"));
  ui->actionToggle_Selected->setIcon(QIcon::fromTheme("edit-select-invert"));
  ui->file_list->addActions({ ui->actionKeep_All, ui->actionKeep_None, ui->actionToggle_Selected });
  ui->file_list->setContextMenuPolicy(Qt::ActionsContextMenu);
}

ExternalChangesDialog::~ExternalChangesDialog()
{
  delete ui;
}

void ExternalChangesDialog::setup(int app_id, const ExternalChangesInfo& info)
{
  app_id_ = app_id;
  changes_info_ = info;

  ui->file_list->clear();
  ui->description_label->setText(
    std::format("Some links created by deployer \"{}\" have been overwritten externally.",
                info.deployer_name)
      .c_str());

  for(const auto& [path, mod_id] : info.file_changes)
  {
    auto item = new QListWidgetItem(path.c_str());
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    ui->file_list->addItem(item);
  }
}

void ExternalChangesDialog::on_buttonBox_accepted()
{
  FileChangeChoices changes_to_keep;

  for(int i = 0; i < ui->file_list->count(); i++)
  {
    const auto& [path, mod_id] = changes_info_.file_changes.at(i);
    changes_to_keep.paths.push_back(path);
    changes_to_keep.mod_ids.push_back(mod_id);
    changes_to_keep.changes_to_keep.push_back(ui->file_list->item(i)->checkState() == Qt::Checked);
  }
  emit externalChangesDialogCompleted(app_id_, changes_info_.deployer_id, changes_to_keep);
}

void ExternalChangesDialog::on_buttonBox_rejected()
{
  emit externalChangesDialogAborted();
}

void ExternalChangesDialog::on_actionKeep_All_triggered()
{
  for(int i = 0; i < ui->file_list->count(); i++)
    ui->file_list->item(i)->setCheckState(Qt::Checked);
}

void ExternalChangesDialog::on_actionKeep_None_triggered()
{
  for(int i = 0; i < ui->file_list->count(); i++)
    ui->file_list->item(i)->setCheckState(Qt::Unchecked);
}

void ExternalChangesDialog::on_actionToggle_Selected_triggered()
{
  for(auto item : ui->file_list->selectedItems())
    item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
}
