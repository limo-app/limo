#include "addapikeydialog.h"
#include "ui_addapikeydialog.h"
#include <QPushButton>


AddApiKeyDialog::AddApiKeyDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AddApiKeyDialog)
{
  ui->setupUi(this);
  ui->pw_field->setPartnerField(ui->pw_repeat_field, PasswordField::repeat);
  connect(ui->pw_repeat_field,
          &PasswordField::passwordValidityChanged,
          this,
          &AddApiKeyDialog::onPasswordValidityChanged);
}

AddApiKeyDialog::~AddApiKeyDialog()
{
  delete ui;
}

QString AddApiKeyDialog::getApiKey() const
{
  return ui->key_field->text();
}

QString AddApiKeyDialog::getPassword() const
{
  return ui->pw_field->getPassword();
}

void AddApiKeyDialog::on_buttonBox_rejected()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  reject();
}

void AddApiKeyDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  accept();
}

void AddApiKeyDialog::onPasswordValidityChanged(bool is_valid)
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(is_valid);
}
