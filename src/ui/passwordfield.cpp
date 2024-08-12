#include "passwordfield.h"
#include <QDebug>
#include <QHBoxLayout>


PasswordField::PasswordField(QWidget* parent) : QWidget{ parent }
{
  auto layout = new QHBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  password_line_edit_ = new ValidatingLineEdit(this, ValidatingLineEdit::VALID_NONE);
  password_line_edit_->setEchoMode(QLineEdit::Password);
  password_line_edit_->setPlaceholderText("Enter Password");
  connect(password_line_edit_, &QLineEdit::textEdited, this, &PasswordField::onPasswordEdited);
  layout->addWidget(password_line_edit_);
  view_button_ = new QPushButton(this);
  view_button_->setText("");
  view_button_->setIcon(show_icon);
  view_button_->setToolTip("Show Password");
  connect(view_button_, &QPushButton::pressed, this, &PasswordField::onViewButtonPressed);
  layout->addWidget(view_button_);
  setLayout(layout);
}

QString PasswordField::getPassword() const
{
  return password_line_edit_->text();
}

ValidatingLineEdit* PasswordField::getPasswordLineEdit() const
{
  return password_line_edit_;
}

void PasswordField::setPartnerField(PasswordField* partner, Role partner_role)
{
  partner_ = partner;
  if(partner_role == main)
  {
    role_ = repeat;
    password_line_edit_->setCustomValidator([partner, this](auto pw)
                                            { return pw == partner->getPassword(); });
    password_line_edit_->setValidationMode(ValidatingLineEdit::VALID_CUSTOM);
    password_line_edit_->setPlaceholderText("Repeat Password");
    connect(partner, &PasswordField::passwordEdited, this, &PasswordField::onPartnerFieldChanged);
  }
  else
  {
    role_ = main;
    password_line_edit_->setValidationMode(ValidatingLineEdit::VALID_NONE);
    password_line_edit_->setPlaceholderText("Enter Password");
    partner->setPartnerField(this, main);
  }
}

void PasswordField::updateValidationStatus()
{
  if(role_ == main)
    return;

  const bool is_valid_now = password_line_edit_->hasValidText();
  if(is_valid_ != is_valid_now)
  {
    is_valid_ = is_valid_now;
    emit passwordValidityChanged(is_valid_now);
  }
}

void PasswordField::onPartnerFieldChanged(QString new_password)
{
  if(role_ == repeat)
    password_line_edit_->updateValidation();
  updateValidationStatus();
}

void PasswordField::onPasswordEdited(QString new_password)
{
  updateValidationStatus();
  emit passwordEdited(new_password);
}

void PasswordField::onViewButtonPressed()
{
  const auto mode = password_line_edit_->echoMode();
  if(mode == QLineEdit::Password)
  {
    password_line_edit_->setEchoMode(QLineEdit::Normal);
    view_button_->setToolTip("Hide Password");
    view_button_->setIcon(hide_icon);
  }
  else
  {
    password_line_edit_->setEchoMode(QLineEdit::Password);
    view_button_->setToolTip("Show Password");
    view_button_->setIcon(show_icon);
  }
}
