#include "addbackuptargetdialog.h"
#include "ui_addbackuptargetdialog.h"
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QPushButton>
#include <QStandardPaths>


AddBackupTargetDialog::AddBackupTargetDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::AddBackupTargetDialog)
{
  ui->setupUi(this);
  file_dialog_ = std::make_unique<QFileDialog>();
  file_dialog_->setOption(QFileDialog::DontUseNativeDialog);
  file_dialog_->setWindowTitle("Select Backup Target");
  connect(file_dialog_.get(),
          &QFileDialog::fileSelected,
          this,
          &AddBackupTargetDialog::onFileDialogAccepted);
  connect(file_dialog_.get(),
          &QFileDialog::currentChanged,
          this,
          &AddBackupTargetDialog::onFileDialogSelectionChanged);
  updateOkButton();
  ui->target_path_field->setValidationMode(ValidatingLineEdit::VALID_PATH_EXISTS);
  dialog_completed_ = false;
}

AddBackupTargetDialog::~AddBackupTargetDialog()
{
  delete ui;
}

void AddBackupTargetDialog::resetDialog(int app_id)
{
  app_id_ = app_id;
  ui->target_name_field->setText("");
  ui->target_path_field->setText("");
  ui->default_backup_field->setText("");
  ui->first_backup_field->setText("");
  updateOkButton();
  dialog_completed_ = false;
}

void AddBackupTargetDialog::updateOkButton()
{
  ui->buttonBox->button(QDialogButtonBox::Ok)
    ->setEnabled(!ui->target_name_field->text().isEmpty() && pathIsValid() &&
                 !ui->default_backup_field->text().isEmpty());
}

bool AddBackupTargetDialog::pathIsValid()
{
  const QString path = ui->target_path_field->text();
  if(path.isEmpty())
    return false;
  return QFileInfo::exists(path);
}

void AddBackupTargetDialog::on_target_name_field_textEdited(const QString& text)
{
  updateOkButton();
}

void AddBackupTargetDialog::on_target_path_field_textEdited(const QString& text)
{
  updateOkButton();
}

void AddBackupTargetDialog::on_default_backup_field_textEdited(const QString& text)
{
  updateOkButton();
}

void AddBackupTargetDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit backupTargetAdded(app_id_,
                         ui->target_name_field->text(),
                         ui->target_path_field->text(),
                         ui->default_backup_field->text(),
                         ui->first_backup_field->text());
}


void AddBackupTargetDialog::on_path_picker_button_clicked()
{
  QString starting_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  if(pathIsValid())
    starting_dir = ui->target_path_field->text();
  file_dialog_->setFileMode(QFileDialog::ExistingFile);
  file_dialog_->setDirectory(starting_dir);
  file_dialog_->exec();
}

void AddBackupTargetDialog::onFileDialogAccepted(const QString& path)
{
  if(!path.isEmpty())
    ui->target_path_field->setText(path);
  updateOkButton();
}

void AddBackupTargetDialog::onFileDialogSelectionChanged(const QString& path)
{
  QFileInfo info(path);
  if(info.isFile())
    file_dialog_->setFileMode(QFileDialog::ExistingFile);
  else if(info.isDir())
    file_dialog_->setFileMode(QFileDialog::Directory);
}
