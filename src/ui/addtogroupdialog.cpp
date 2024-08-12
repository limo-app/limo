#include "addtogroupdialog.h"
#include "ui_addtogroupdialog.h"
#include <QApplication>
#include <QPushButton>


AddToGroupDialog::AddToGroupDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AddToGroupDialog)
{
  ui->setupUi(this);
  ui->group_field->setCustomValidator([groups = &groups_](QString s)
                                      { return groups->contains(s); });
  ui->group_field->setValidationMode(ValidatingLineEdit::VALID_CUSTOM);
}

AddToGroupDialog::~AddToGroupDialog()
{
  delete ui;
}

void AddToGroupDialog::setupDialog(const QStringList& groups,
                                   const std::vector<int>& mod_ids,
                                   const QString& mod_name,
                                   int mod_id)
{
  groups_ = groups;
  mod_id_ = mod_id;
  mod_ids_ = mod_ids;
  ui->label->setText("Add \"" + mod_name + "\" to:");
  completer_ = std::make_unique<QCompleter>(groups);
  completer_->setCaseSensitivity(Qt::CaseInsensitive);
  completer_->setFilterMode(Qt::MatchContains);
  ui->group_field->setCompleter(completer_.get());
  ui->group_field->clear();
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  dialog_completed_ = false;
}

void AddToGroupDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  const QString group = ui->group_field->text();
  if(groups_.contains(group))
    emit modAddedToGroup(mod_id_, mod_ids_[groups_.indexOf(group)]);
}

void AddToGroupDialog::on_group_field_textChanged(const QString& text)
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->group_field->hasValidText());
}
