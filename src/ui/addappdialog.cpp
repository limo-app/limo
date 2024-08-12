#include "addappdialog.h"
#include "importfromsteamdialog.h"
#include "ui_addappdialog.h"
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <filesystem>

namespace sfs = std::filesystem;


AddAppDialog::AddAppDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AddAppDialog)
{
  ui->setupUi(this);
  ui->move_dir_box->setVisible(false);
  ui->import_checkbox->setVisible(false);
  enableOkButton(false);
  ui->path_field->setValidationMode(ValidatingLineEdit::VALID_PATH_EXISTS);
  dialog_completed_ = false;
}

AddAppDialog::~AddAppDialog()
{
  delete ui;
}

void AddAppDialog::on_file_picker_button_clicked()
{
  QString starting_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  if(pathIsValid())
    starting_dir = ui->path_field->text();
  auto dialog = new QFileDialog;
  dialog->setWindowTitle("Select Staging Directory");
  dialog->setFilter(QDir::AllDirs | QDir::Hidden);
  dialog->setFileMode(QFileDialog::Directory);
  dialog->setDirectory(starting_dir);
  connect(dialog, &QFileDialog::fileSelected, this, &AddAppDialog::onFileDialogAccepted);
  dialog->exec();
}

void AddAppDialog::on_name_field_textChanged(const QString& text)
{
  if(text.isEmpty())
    enableOkButton(false);
  else if(pathIsValid())
    enableOkButton(true);
}

void AddAppDialog::on_path_field_textChanged(const QString& text)
{
  if(!pathIsValid())
    enableOkButton(false);
  else if(!ui->name_field->text().isEmpty())
    enableOkButton(true);
}

void AddAppDialog::enableOkButton(bool state)
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(state);
}

bool AddAppDialog::pathIsValid()
{
  QString path = ui->path_field->text();
  if(path.isEmpty())
    return false;
  return std::filesystem::exists(path.toStdString());
}

bool AddAppDialog::iconIsValid(const QString& path)
{
  QString icon_path = path.isEmpty() ? ui->icon_field->text() : path;
  return QIcon(icon_path).availableSizes().size() > 0;
}

void AddAppDialog::setEditMode(const QString& name,
                               const QString& app_version,
                               const QString& path,
                               const QString& command,
                               const QString& icon_path,
                               int app_id)
{
  steam_prefix_path_ = "";
  steam_install_path_ = "";
  ui->import_checkbox->setVisible(false);
  ui->import_button->setEnabled(false);
  ui->import_button->setHidden(true);
  ui->move_dir_box->setCheckState(Qt::Unchecked);
  name_ = name;
  path_ = path;
  command_ = command;
  app_id_ = app_id;
  enableOkButton(true);
  edit_mode_ = true;
  ui->move_dir_box->setVisible(true);
  setWindowTitle("Edit " + name_);
  ui->name_field->setText(name);
  ui->version_field->setText(app_version);
  ui->icon_field->setText(icon_path);
  if(iconIsValid(icon_path))
    ui->icon_picker_button->setIcon(QIcon(icon_path));
  else
    ui->icon_picker_button->setIcon(QIcon::fromTheme("folder-open"));
  ui->path_field->setText(path);
  ui->command_field->setText(command);
  dialog_completed_ = false;
}

void AddAppDialog::setAddMode()
{
  steam_prefix_path_ = "";
  steam_install_path_ = "";
  ui->import_checkbox->setVisible(false);
  ui->import_button->setEnabled(true);
  ui->import_button->setHidden(false);
  setWindowTitle("New Application");
  ui->name_field->setText("");
  ui->version_field->setText("");
  ui->icon_field->setText("");
  ui->icon_picker_button->setIcon(QIcon::fromTheme("folder-open"));
  ui->path_field->setText("");
  ui->command_field->setText("");
  enableOkButton(false);
  edit_mode_ = false;
  ui->move_dir_box->setVisible(false);
  dialog_completed_ = false;
}

void AddAppDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  EditApplicationInfo info;
  info.name = ui->name_field->text().toStdString();
  info.app_version = ui->version_field->text().toStdString();
  info.staging_dir = ui->path_field->text().toStdString();
  info.command = ui->command_field->text().toStdString();
  info.icon_path = ui->icon_field->text().toStdString();
  if(edit_mode_)
  {
    info.move_staging_dir = ui->move_dir_box->checkState() == Qt::Checked;
    emit applicationEdited(info, app_id_);
  }
  else
  {
    info.deployers = std::vector<std::pair<std::string, std::string>>{};
    if(ui->import_checkbox->isChecked())
    {
      if(steam_install_path_ != "")
        info.deployers.push_back({ "Install", steam_install_path_.toStdString() });
      if(steam_prefix_path_ != "")
        info.deployers.push_back({ "Prefix", steam_prefix_path_.toStdString() });
    }
    emit applicationAdded(info);
  }
}

void AddAppDialog::on_import_button_clicked()
{
  auto dialog = new ImportFromSteamDialog(this);
  connect(dialog,
          &ImportFromSteamDialog::applicationImported,
          this,
          &AddAppDialog::onApplicationImported);
  dialog->exec();
}

void AddAppDialog::onApplicationImported(QString name,
                                         QString app_id,
                                         QString install_dir,
                                         QString prefix_path,
                                         QString icon_path)
{
  ui->name_field->setText(name);
  ui->command_field->setText("steam -applaunch " + app_id);
  ui->import_checkbox->setVisible(true);
  steam_install_path_ = install_dir;
  steam_prefix_path_ = prefix_path;
  ui->icon_field->setText(icon_path);
  ui->icon_picker_button->setIcon(QIcon(icon_path));
}

void AddAppDialog::onFileDialogAccepted(const QString& path)
{
  if(!path.isEmpty())
    ui->path_field->setText(path);
}

void AddAppDialog::on_icon_picker_button_clicked()
{
  QString starting_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  QString path = ui->icon_field->text();
  if(!path.isEmpty() && std::filesystem::exists(path.toStdString()))
    starting_dir = std::filesystem::path(path.toStdString()).parent_path().string().c_str();
  auto dialog = new QFileDialog;
  dialog->setWindowTitle("Select Icon");
  dialog->setFilter(QDir::AllDirs | QDir::Hidden);
  dialog->setDirectory(starting_dir);
  connect(dialog, &QFileDialog::fileSelected, this, &AddAppDialog::onIconPathDialogComplete);
  dialog->exec();
}

void AddAppDialog::onIconPathDialogComplete(const QString& path)
{
  if(!iconIsValid(path))
  {
    QMessageBox* error_box =
      new QMessageBox(QMessageBox::Critical, "Error", "Invalid icon!", QMessageBox::Ok);
    error_box->exec();
    return;
  }
  ui->icon_field->setText(path);
  ui->icon_picker_button->setIcon(QIcon(path));
}
