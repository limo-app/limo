#include "editmodsourcesdialog.h"
#include "ui_editmodsourcesdialog.h"
#include <QStandardPaths>


EditModSourcesDialog::EditModSourcesDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::EditModSourcesDialog)
{
  ui->setupUi(this);
  ui->local_source_field->setValidationMode(ValidatingLineEdit::VALID_PATH_EXISTS);
  file_dialog_ = std::make_unique<QFileDialog>();
  file_dialog_->setOption(QFileDialog::DontUseNativeDialog);
  file_dialog_->setWindowTitle("Select Local Source");
  connect(file_dialog_.get(),
          &QFileDialog::fileSelected,
          this,
          &EditModSourcesDialog::onFileDialogAccepted);
  connect(file_dialog_.get(),
          &QFileDialog::currentChanged,
          this,
          &EditModSourcesDialog::onFileDialogSelectionChanged);
}

EditModSourcesDialog::~EditModSourcesDialog()
{
  delete ui;
}

void EditModSourcesDialog::setupDialog(int app_id,
                                       int mod_id,
                                       const QString& mod_name,
                                       const QString& local_source,
                                       const QString& remote_source)
{
  app_id_ = app_id;
  mod_id_ = mod_id;
  ui->info_label->setText("Edit sources for \"" + mod_name + "\"");
  ui->local_source_field->setText(local_source);
  ui->remote_source_field->setText(remote_source);
  dialog_completed_ = false;
}

void EditModSourcesDialog::closeEvent(QCloseEvent* event)
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit dialogClosed();
  QDialog::closeEvent(event);
}

void EditModSourcesDialog::onFileDialogAccepted(const QString& path)
{
  if(!path.isEmpty())
    ui->local_source_field->setText(path);
}

void EditModSourcesDialog::onFileDialogSelectionChanged(const QString& path)
{
  QFileInfo info(path);
  if(info.isFile())
    file_dialog_->setFileMode(QFileDialog::ExistingFile);
  else if(info.isDir())
    file_dialog_->setFileMode(QFileDialog::Directory);
}

void EditModSourcesDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  emit modSourcesEdited(
    app_id_, mod_id_, ui->local_source_field->text(), ui->remote_source_field->text());
}

void EditModSourcesDialog::on_buttonBox_rejected()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  emit dialogClosed();
}

void EditModSourcesDialog::on_path_picker_button_clicked()
{
  QString starting_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  // if(QFileInfo::exists(ui->local_source_field->text()))
  //   starting_dir = ui->local_source_field->text();
  file_dialog_->setFileMode(QFileDialog::ExistingFile);
  file_dialog_->setDirectory(starting_dir);
  file_dialog_->exec();
}
