#include "addautotagdialog.h"
#include "ui_addautotagdialog.h"
#include <QPushButton>


AddAutoTagDialog::AddAutoTagDialog(const QStringList& existing_tags, QWidget* parent) :
  QDialog(parent), ui(new Ui::AddAutoTagDialog)
{
  ui->setupUi(this);
  ui->name_field->setCustomValidator([existing_tags](const auto& name)
                                     { return !name.isEmpty() && !existing_tags.contains(name); });
  ui->name_field->setValidationMode(ValidatingLineEdit::VALID_CUSTOM);
  connect(ui->name_field, &QLineEdit::textEdited, this, &AddAutoTagDialog::onTagNameEdited);
  setWindowTitle("Add Auto Tag");
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

AddAutoTagDialog::AddAutoTagDialog(QStringList existing_tags,
                                   const QString& tag_name,
                                   QWidget* parent) : QDialog(parent), ui(new Ui::AddAutoTagDialog)
{
  ui->setupUi(this);
  existing_tags.removeAll(tag_name);
  ui->name_field->setCustomValidator([existing_tags](const auto& name)
                                     { return !name.isEmpty() && !existing_tags.contains(name); });
  ui->name_field->setValidationMode(ValidatingLineEdit::VALID_CUSTOM);
  ui->name_field->setText(tag_name);
  connect(ui->name_field, &QLineEdit::textEdited, this, &AddAutoTagDialog::onTagNameEdited);
  setWindowTitle("Rename Auto Tag");
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

AddAutoTagDialog::~AddAutoTagDialog()
{
  delete ui;
}

QString AddAutoTagDialog::getName() const
{
  return ui->name_field->text();
}

void AddAutoTagDialog::onTagNameEdited()
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->name_field->hasValidText());
}
