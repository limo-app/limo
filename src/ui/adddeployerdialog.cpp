#include "adddeployerdialog.h"
#include "../core/deployerfactory.h"
#include "qmessagebox.h"
#include "ui_adddeployerdialog.h"
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>


AddDeployerDialog::AddDeployerDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::AddDeployerDialog)
{
  ui->setupUi(this);
  setupTypeBox();
  enableOkButton(false);
  ui->path_field->setValidationMode(ValidatingLineEdit::VALID_PATH_EXISTS);
  ui->source_path_field->setValidationMode(ValidatingLineEdit::VALID_PATH_EXISTS);
}

AddDeployerDialog::~AddDeployerDialog()
{
  delete ui;
}

void AddDeployerDialog::enableOkButton(bool state)
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(state);
}

bool AddDeployerDialog::pathIsValid()
{
  QString path = ui->path_field->text();
  if(path.isEmpty())
    return false;
  return QDir(path).exists();
}

void AddDeployerDialog::setupTypeBox()
{
  ui->type_box->clear();
  for(const auto& type : DeployerFactory::DEPLOYER_TYPES)
  {
    ui->type_box->addItem(type.c_str());
    ui->type_box->setItemData(ui->type_box->count() - 1,
                              DeployerFactory::DEPLOYER_DESCRIPTIONS.at(type).c_str(),
                              Qt::ToolTipRole);
  }
}

void AddDeployerDialog::updateOkButton()
{
  enableOkButton(ui->name_field->hasValidText() && ui->path_field->hasValidText() &&
                 ui->source_path_field->hasValidText());
}

void AddDeployerDialog::setAddMode(int app_id)
{
  app_id_ = app_id;
  setWindowTitle("New Deployer");
  ui->name_field->clear();
  ui->path_field->clear();
  ui->source_path_field->clear();
  ui->deploy_mode_box->setCurrentIndex(Deployer::hard_link);
  ui->warning_label->setHidden(true);
  ui->sym_link_label->setHidden(true);
  edit_mode_ = false;
  setupTypeBox();
  enableOkButton(false);
  updateSourceFields();
  ui->name_field->updateValidation();
  ui->path_field->updateValidation();
  ui->source_path_field->updateValidation();
  dialog_completed_ = false;
  ui->rev_depl_ignore_cb->setText("Ignore current files");
  disable_confirmation_boxes_ = true;
  ui->rev_depl_ignore_cb->setCheckState(Qt::Unchecked);
  ui->rev_depl_separate_cb->setCheckState(Qt::Unchecked);
  disable_confirmation_boxes_ = false;
}

void AddDeployerDialog::setEditMode(const QString& type,
                                    const QString& name,
                                    const QString& target_path,
                                    const QString& source_path,
                                    Deployer::DeployMode deploy_mode,
                                    int app_id,
                                    int deployer_id,
                                    bool has_separate_dirs,
                                    bool has_ignored_files)
{
  name_ = name;
  target_path_ = target_path;
  source_path_ = source_path;
  type_ = type;
  app_id_ = app_id;
  deployer_id_ = deployer_id;
  has_separate_dirs_ = has_separate_dirs;
  has_ignored_files_ = has_ignored_files;
  ui->deploy_mode_box->setCurrentIndex(deploy_mode);
  ui->warning_label->setHidden(deploy_mode != Deployer::copy);
  ui->sym_link_label->setHidden(deploy_mode != Deployer::sym_link);
  setupTypeBox();
  setWindowTitle("Edit " + name);
  edit_mode_ = true;
  ui->name_field->setText(name);
  ui->path_field->setText(target_path);
  for(int i = 0; i < ui->type_box->count(); i++)
  {
    if(ui->type_box->itemText(i) == type)
      ui->type_box->setCurrentIndex(i);
  }
  if(DeployerFactory::AUTONOMOUS_DEPLOYERS.at(ui->type_box->currentText().toStdString()))
    ui->source_path_field->setText(source_path);
  else
    ui->source_path_field->clear();
  updateSourceFields();
  ui->name_field->updateValidation();
  ui->path_field->updateValidation();
  ui->source_path_field->updateValidation();
  dialog_completed_ = false;
  ui->rev_depl_ignore_cb->setText(has_ignored_files ? "Use ignore list" : "Ignore current files");
  disable_confirmation_boxes_ = true;
  ui->rev_depl_ignore_cb->setCheckState(has_ignored_files ? Qt::Checked : Qt::Unchecked);
  ui->rev_depl_separate_cb->setCheckState(has_separate_dirs ? Qt::Checked : Qt::Unchecked);
  disable_confirmation_boxes_ = false;
}

void AddDeployerDialog::updateSourceFields()
{
  const std::string cur_text = ui->type_box->currentText().toStdString();
  if(cur_text.empty())
    return;
  const bool is_reverse_deployer = cur_text == DeployerFactory::REVERSEDEPLOYER;
  const bool hide_source =
    !DeployerFactory::AUTONOMOUS_DEPLOYERS.at(cur_text) || is_reverse_deployer;
  const bool hide_mode = !hide_source && !is_reverse_deployer;
  ui->source_path_field->setHidden(hide_source);
  ui->source_dir_label->setHidden(hide_source);
  ui->source_picker_button->setHidden(hide_source);
  ui->source_path_field->updateValidation();
  ui->deploy_mode_box->setHidden(hide_mode);
  ui->method_label->setHidden(hide_mode);
  const int mode_index = ui->deploy_mode_box->currentIndex();
  ui->sym_link_label->setHidden(mode_index != Deployer::sym_link || hide_mode);
  ui->warning_label->setHidden(mode_index != Deployer::copy || hide_mode);
  ui->rev_depl_separate_cb->setHidden(!is_reverse_deployer);
  ui->rev_depl_ignore_cb->setHidden(!is_reverse_deployer);
  ui->rev_depl_ignore_button->setHidden(!is_reverse_deployer || !edit_mode_ ||
                                        type_.toStdString() != DeployerFactory::REVERSEDEPLOYER ||
                                        !has_ignored_files_);
  updateOkButton();
}

void AddDeployerDialog::on_file_picker_button_clicked()
{
  QString starting_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  if(pathIsValid())
    starting_dir = ui->path_field->text();
  auto dialog = new QFileDialog;
  dialog->setWindowTitle("Select Target Directory");
  dialog->setFilter(QDir::AllDirs | QDir::Hidden);
  dialog->setFileMode(QFileDialog::Directory);
  dialog->setDirectory(starting_dir);
  connect(dialog, &QFileDialog::fileSelected, this, &AddDeployerDialog::onFileDialogAccepted);
  dialog->exec();
}


void AddDeployerDialog::on_name_field_textChanged(const QString& text)
{
  updateOkButton();
}


void AddDeployerDialog::on_path_field_textChanged(const QString& text)
{
  updateOkButton();
}


void AddDeployerDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  EditDeployerInfo info;
  info.type = ui->type_box->currentText().toStdString();
  info.name = ui->name_field->text().toStdString();
  info.target_dir = ui->path_field->text().toStdString();
  info.source_dir = ui->source_path_field->text().toStdString();
  info.deploy_mode = static_cast<Deployer::DeployMode>(ui->deploy_mode_box->currentIndex());
  info.separate_profile_dirs = ui->rev_depl_separate_cb->checkState() == Qt::Checked;
  info.update_ignore_list = ui->rev_depl_ignore_cb->checkState() == Qt::Checked;
  if(edit_mode_)
    emit deployerEdited(info, app_id_, deployer_id_);
  else
    emit deployerAdded(info, app_id_);
}

void AddDeployerDialog::onFileDialogAccepted(const QString& path)
{
  if(!path.isEmpty())
    ui->path_field->setText(path);
}

void AddDeployerDialog::on_type_box_currentIndexChanged(int index)
{
  updateSourceFields();
}

void AddDeployerDialog::on_source_picker_button_clicked()
{
  QString starting_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  QString path = ui->source_path_field->text();
  if(!path.isEmpty() && QDir(path).exists())
    starting_dir = path;
  auto dialog = new QFileDialog;
  dialog->setWindowTitle("Select Source Directory");
  dialog->setFilter(QDir::AllDirs | QDir::Hidden);
  dialog->setFileMode(QFileDialog::Directory);
  dialog->setDirectory(starting_dir);
  connect(dialog, &QFileDialog::fileSelected, this, &AddDeployerDialog::onSourceDialogAccepted);
  dialog->exec();
}

void AddDeployerDialog::onSourceDialogAccepted(const QString& path)
{
  if(!path.isEmpty())
    ui->source_path_field->setText(path);
}


void AddDeployerDialog::on_source_path_field_textChanged(const QString& path)
{
  updateOkButton();
}

void AddDeployerDialog::on_deploy_mode_box_currentIndexChanged(int index)
{
  ui->warning_label->setHidden(index != Deployer::copy);
  ui->sym_link_label->setHidden(index != Deployer::sym_link);
}

void AddDeployerDialog::on_rev_depl_ignore_cb_stateChanged(int new_state)
{
  if(!edit_mode_ || new_state == Qt::Checked || !has_ignored_files_)
    return;

  QMessageBox box;
  box.setWindowTitle("Confirm deletion");
  box.setText("This will delete the current ignore list.\nThis action cannot be undone.");
  box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  int ret = box.exec();
  if(ret != QMessageBox::Ok)
    ui->rev_depl_ignore_cb->setCheckState(Qt::Checked);
}

void AddDeployerDialog::on_rev_depl_separate_cb_stateChanged(int new_state)
{
  if(!edit_mode_ || new_state == Qt::Checked || !has_separate_dirs_)
    return;

  QMessageBox box;
  box.setWindowTitle("Confirm deletion");
  box.setText("This will delete all files except those managed by the current profile.\n"
              "This action cannot be undone.");
  box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  int ret = box.exec();
  if(ret != QMessageBox::Ok)
    ui->rev_depl_separate_cb->setCheckState(Qt::Checked);
}

void AddDeployerDialog::on_rev_depl_ignore_button_clicked()
{
  emit updateIgnoredFiles(app_id_, deployer_id_);
}

