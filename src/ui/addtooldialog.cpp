#include "addtooldialog.h"
#include "ui_addtooldialog.h"
#include <QPushButton>

AddToolDialog::AddToolDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AddToolDialog)
{
  ui->setupUi(this);
  updateOkButton();
}

AddToolDialog::~AddToolDialog()
{
  delete ui;
}

void AddToolDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  emit toolAdded(ui->name_field->text(), ui->command_field->text());
}

void AddToolDialog::updateOkButton()
{
  if(ui->command_field->text().isEmpty() || ui->name_field->text().isEmpty())
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  else
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void AddToolDialog::setupDialog()
{
  ui->command_field->setText("");
  ui->name_field->setText("");
  updateOkButton();
  dialog_completed_ = false;
}

void AddToolDialog::on_name_field_textChanged(const QString& text)
{
  updateOkButton();
}


void AddToolDialog::on_command_field_textChanged(const QString& text)
{
  updateOkButton();
}
