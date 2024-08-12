#include "managemodtagsdialog.h"
#include "ui_managemodtagsdialog.h"

ManageModTagsDialog::ManageModTagsDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::ManageModTagsDialog)
{
  ui->setupUi(this);
  ui->mode_box->addItems({ "Add", "Remove", "Overwrite" });
  setWindowTitle("Edit Tags");
}

ManageModTagsDialog::~ManageModTagsDialog()
{
  delete ui;
}

void ManageModTagsDialog::setupDialog(int app_id,
                                      const QStringList& tags,
                                      const QStringList& mod_tags,
                                      const QString& mod_name,
                                      const std::vector<int>& mod_ids)
{
  app_id_ = app_id;
  mod_name_ = mod_name;
  mod_ids_ = mod_ids;
  ui->mode_box->setCurrentIndex(0);
  updateHintText();
  ui->tag_list->clear();
  for(const auto& tag : tags)
  {
    auto item = new QListWidgetItem(tag);
    item->setCheckState(mod_tags.contains(tag) ? Qt::Checked : Qt::Unchecked);
    ui->tag_list->addItem(item);
  }
  dialog_completed_ = false;
}

void ManageModTagsDialog::closeEvent(QCloseEvent* event)
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit dialogClosed();
  QDialog::closeEvent(event);
}

void ManageModTagsDialog::updateHintText()
{
  QString suffix =
    mod_ids_.size() == 1 ? "\"" + mod_name_ + "\":" : QString::number(mod_ids_.size()) + " mods:";
  if(ui->mode_box->currentIndex() == add_mode)
    ui->text_label->setText("Add tags to " + suffix);
  else if(ui->mode_box->currentIndex() == remove_mode)
    ui->text_label->setText("Remove tags from " + suffix);
  else
    ui->text_label->setText("Overwrite tags for " + suffix);
}

void ManageModTagsDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  QStringList tags;
  for(int i = 0; i < ui->tag_list->count(); i++)
  {
    if(ui->tag_list->item(i)->checkState() == Qt::Checked)
      tags.append(ui->tag_list->item(i)->text());
  }
  emit modTagsUpdated(app_id_, tags, mod_ids_, ui->mode_box->currentIndex());
}


void ManageModTagsDialog::on_buttonBox_rejected()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  emit dialogClosed();
}

void ManageModTagsDialog::on_mode_box_currentIndexChanged(int index)
{
  updateHintText();
}
