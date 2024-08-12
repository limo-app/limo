#include "changeapipwdialog.h"
#include "../core/cryptography.h"
#include "../core/log.h"
#include "../core/nexus/api.h"
#include "ui_changeapipwdialog.h"
#include <QMessageBox>


ChangeApiPwDialog::ChangeApiPwDialog(bool uses_default_pw,
                                     const std::string& cipher,
                                     const std::string& nonce,
                                     const std::string& tag,
                                     QWidget* parent) :
  uses_default_pw_(uses_default_pw), cipher_(cipher), nonce_(nonce), tag_(tag), QDialog(parent),
  ui(new Ui::ChangeApiPwDialog)
{
  ui->setupUi(this);
  ui->new_pw_field->setPartnerField(ui->repeat_pw_field, PasswordField::repeat);
  ui->error_label->setVisible(false);
  if(nexus::Api::isInitialized() || uses_default_pw_)
  {
    ui->enter_old_label->setVisible(false);
    ui->old_pw_field->setVisible(false);
  }
  connect(ui->repeat_pw_field,
          &PasswordField::passwordValidityChanged,
          this,
          &ChangeApiPwDialog::onPasswordValidityChanged);
  uses_default_pw_ = uses_default_pw;
}

ChangeApiPwDialog::~ChangeApiPwDialog()
{
  delete ui;
}

void ChangeApiPwDialog::onPasswordValidityChanged(bool is_valid)
{
  ui->button_box->button(QDialogButtonBox::Ok)->setEnabled(is_valid);
}

void ChangeApiPwDialog::on_button_box_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  std::string api_key;
  if(!nexus::Api::isInitialized())
  {
    const std::string password = uses_default_pw_ || ui->old_pw_field->getPassword().isEmpty()
                                   ? cryptography::default_key
                                   : ui->old_pw_field->getPassword().toStdString();
    try
    {
      api_key = cryptography::decrypt(cipher_, password, nonce_, tag_);
    }
    catch(CryptographyError& e)
    {
      ui->error_label->setVisible(true);
      dialog_completed_ = false;
      return;
    }
    nexus::Api::setApiKey(api_key);
  }
  else
    api_key = nexus::Api::getApiKey();

  std::tuple<std::string, std::string, std::string> res;
  try
  {
    res = cryptography::encrypt(api_key, ui->new_pw_field->getPassword().toStdString());
  }
  catch(CryptographyError& e)
  {
    const QString message = "Error during key encryption.";
    Log::error(message.toStdString());
    QMessageBox error_box(QMessageBox::Critical, "Error", message, QMessageBox::Ok);
    error_box.exec();
    dialog_completed_ = false;
    return;
  }
  const auto [cipher, nonce, tag] = res;
  emit keyEncryptionUpdated(cipher, nonce, tag, ui->new_pw_field->getPassword().isEmpty());
  accept();
}


void ChangeApiPwDialog::on_button_box_rejected()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  reject();
}
