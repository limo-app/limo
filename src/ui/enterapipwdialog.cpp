#include "enterapipwdialog.h"
#include "core/cryptography.h"
#include "ui_enterapipwdialog.h"


EnterApiPwDialog::EnterApiPwDialog(const std::string& cipher,
                                   const std::string& nonce,
                                   const std::string& tag,
                                   QWidget* parent) :
  cipher_(cipher), nonce_(nonce), tag_(tag), QDialog(parent), ui(new Ui::EnterApiPwDialog)
{
  ui->setupUi(this);
  ui->error_label->setVisible(false);
}

EnterApiPwDialog::~EnterApiPwDialog()
{
  delete ui;
}

std::string EnterApiPwDialog::getApiKey() const
{
  return api_key_;
}

bool EnterApiPwDialog::wasSuccessful() const
{
  return success_;
}

void EnterApiPwDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  try
  {
    api_key_ =
      cryptography::decrypt(cipher_, ui->pw_field->getPassword().toStdString(), nonce_, tag_);
  }
  catch(CryptographyError& e)
  {
    ui->error_label->setVisible(true);
    dialog_completed_ = false;
    return;
  }
  success_ = true;
  accept();
}

void EnterApiPwDialog::on_buttonBox_rejected()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  reject();
}
