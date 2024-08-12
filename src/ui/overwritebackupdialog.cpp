#include "overwritebackupdialog.h"
#include "colors.h"
#include "ui_overwritebackupdialog.h"
#include <QPushButton>


OverwriteBackupDialog::OverwriteBackupDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::OverwriteBackupDialog)
{
  ui->setupUi(this);
  ui->backup_field->setCustomValidator([names = &backup_names_](QString s)
                                       { return names->contains(s); });
  ui->backup_field->setValidationMode(ValidatingLineEdit::VALID_CUSTOM);
}

OverwriteBackupDialog::~OverwriteBackupDialog()
{
  delete ui;
}

void OverwriteBackupDialog::setupDialog(const QStringList& backup_names,
                                        int target_id,
                                        int dest_backup)
{
  backup_names_.clear();
  for(int i = 0; i < backup_names.size(); i++)
  {
    if(i != dest_backup)
      backup_names_.append(backup_names[i]);
  }
  backup_target_ = target_id;
  dest_backup_ = dest_backup;
  ui->warning_label->setText("WARNING: All files in '" + backup_names[dest_backup] +
                             "' will be overwritten!");
  QPalette palette = ui->warning_label->palette();
  palette.setColor(QPalette::WindowText, colors::RED);
  ui->warning_label->setPalette(palette);
  completer_ = std::make_unique<QCompleter>(backup_names_);
  completer_->setCaseSensitivity(Qt::CaseInsensitive);
  completer_->setFilterMode(Qt::MatchContains);
  ui->backup_field->setCompleter(completer_.get());
  ui->backup_field->clear();
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  dialog_completed_ = false;
}

void OverwriteBackupDialog::on_backup_field_textChanged(const QString& text)
{
  if(backup_names_.contains(text))
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  else
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void OverwriteBackupDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  const QString backup = ui->backup_field->text();
  if(backup_names_.contains(backup))
  {
    int source = backup_names_.indexOf(backup);
    if(source >= dest_backup_)
      source++;
    emit backupOverwritten(backup_target_, source, dest_backup_);
  }
}
