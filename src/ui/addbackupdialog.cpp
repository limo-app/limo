#include "addbackupdialog.h"
#include "ui_addbackupdialog.h"
#include <QDialogButtonBox>
#include <QPushButton>


AddBackupDialog::AddBackupDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AddBackupDialog)
{
  ui->setupUi(this);
}

AddBackupDialog::~AddBackupDialog()
{
  delete ui;
}

void AddBackupDialog::setupDialog(int app_id,
                                  int target_id,
                                  const QString& target_name,
                                  const QStringList& existing_backups)
{
  target_name_ = target_name;
  app_id_ = app_id;
  target_id_ = target_id;
  this->setWindowTitle("Add backup to " + target_name);
  ui->name_field->clear();
  ui->copy_from_box->clear();
  ui->copy_from_box->addItems(existing_backups);
  dialog_completed_ = false;
}

void AddBackupDialog::on_name_field_textChanged(const QString& text)
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

void AddBackupDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit addBackupDialogAccepted(
    app_id_, target_id_, ui->name_field->text(), target_name_, ui->copy_from_box->currentIndex());
}
