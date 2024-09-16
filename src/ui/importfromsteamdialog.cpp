#include "importfromsteamdialog.h"
#include "../core/log.h"
#include "ui_importfromsteamdialog.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <fstream>
#include <regex>

namespace sfs = std::filesystem;


ImportFromSteamDialog::ImportFromSteamDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::ImportFromSteamDialog)
{
  ui->setupUi(this);
  setWindowTitle("Import App");
  QString path = QSettings(QCoreApplication::applicationName()).value("import/path", "").toString();
  if(!path.isEmpty() && pathIsValid(path.toStdString()))
  {
    ui->path_field->setText(path);
    updateTable(path.toStdString());
  }
  else
  {
    sfs::path default_path =
      QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toStdString();
    default_path = default_path / ".steam" / "steam" / "steamapps";
    if(pathIsValid(default_path))
    {
      ui->path_field->setText(default_path.string().c_str());
      updateTable(default_path);
    }
  }
}

ImportFromSteamDialog::~ImportFromSteamDialog()
{
  delete ui;
}

void ImportFromSteamDialog::on_pick_path_button_clicked()
{
  QString starting_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  if(sfs::exists(ui->path_field->text().toStdString()))
    starting_dir = ui->path_field->text();
  QString path = QFileDialog::getExistingDirectory(
    this, "Select steamapps Directory", starting_dir, QFileDialog::ShowDirsOnly);
  updateTable(path.toStdString());
}

bool ImportFromSteamDialog::pathIsValid(sfs::path path) const
{
  return sfs::is_regular_file(path / "libraryfolders.vdf");
}

void ImportFromSteamDialog::updateTable(sfs::path steam_dir)
{
  ui->app_table->setSortingEnabled(false);
  ui->app_table->setRowCount(0);
  if(!pathIsValid(steam_dir))
  {
    showError("Invalid Path", "Could not find \"libraryfolders.vdf\"!");
    return;
  }
  ui->path_field->setText(steam_dir.c_str());
  QSettings(QCoreApplication::applicationName()).setValue("import/path", steam_dir.c_str());

  std::ifstream file(steam_dir / library_file_name_);
  if(!file.is_open())
  {
    showError("IO Error", "Could not open \"libraryfolders.vdf\"!");
    return;
  }
  std::string line;
  std::regex dir_id_regex("\\s*\"[0-9]+\"");
  std::regex path_regex("\\s*\"path\"\\s*\"([^\"]+)\"");
  std::regex apps_regex("\\s*\"apps\"");
  std::regex app_id_regex("\\s*\"(\\d+)\"\\s*\"\\d+\"");
  std::regex close_brace_regex("\\s*}");
  std::string cur_path = "";
  bool is_in_apps = false;
  while(std::getline(file, line))
  {
    std::smatch match;
    if(std::regex_search(line, match, path_regex))
    {
      cur_path = match[1].str();
      continue;
    }
    if(std::regex_search(line, apps_regex))
    {
      is_in_apps = true;
      continue;
    }
    if(is_in_apps)
    {
      if(std::regex_search(line, close_brace_regex))
      {
        is_in_apps = false;
        continue;
      }
      if(std::regex_search(line, match, app_id_regex))
      {
        if(!addTableRow(match[1].str(), cur_path))
          continue;
      }
    }
  }
  ui->app_table->resizeColumnToContents(0);
  ui->app_table->resizeColumnToContents(1);
  ui->app_table->resizeColumnToContents(2);
  ui->app_table->sortByColumn(0, Qt::AscendingOrder);
  ui->app_table->setSortingEnabled(true);
  ui->search_field->setFocus();
}

bool ImportFromSteamDialog::addTableRow(std::string app_id, sfs::path path)
{
  // Name | App ID | Prefix | Path
  std::string file_name = std::string("appmanifest_") + app_id + ".acf";
  const sfs::path file_path = path / "steamapps" / file_name;
  std::ifstream file(file_path);
  if(!file.is_open())
  {
    Log::warning(std::string("Could not open ") + file_path.c_str());
    return false;
  }
  std::string line;
  std::regex name_regex("\\s*\"name\"\\s*\"([^\"]+)\"");
  std::regex dir_regex("\\s*\"installdir\"\\s*\"([^\"]+)\"");
  std::string name = "";
  std::string install_dir = "";
  std::smatch match;
  while(std::getline(file, line))
  {
    if(std::regex_search(line, match, name_regex))
      name = match[1].str();
    else if(std::regex_search(line, match, dir_regex))
      install_dir = match[1].str();
    if(name != "" && install_dir != "")
      break;
  }
  sfs::path full_path = path / "steamapps" / "common" / install_dir;
  QString has_prefix = "False";
  if(sfs::exists(path / "steamapps" / "compatdata" / app_id))
    has_prefix = "True";
  sfs::path icon_path = path / "appcache" / "librarycache" / (app_id + "_icon.jpg");
  int row = ui->app_table->rowCount();
  ui->app_table->insertRow(row);
  ui->app_table->setItem(
    row, 0, new QTableWidgetItem(QIcon(icon_path.string().c_str()), name.c_str()));
  ui->app_table->item(row, 0)->setData(Qt::UserRole, QString(icon_path.string().c_str()));
  ui->app_table->setItem(row, 1, new QTableWidgetItem(app_id.c_str()));
  ui->app_table->setItem(row, 2, new QTableWidgetItem(has_prefix));
  ui->app_table->setItem(row, 3, new QTableWidgetItem(full_path.c_str()));
  return true;
}

void ImportFromSteamDialog::showError(QString title, QString message)
{
  Log::error(message.toStdString());
  QMessageBox* error_box = new QMessageBox(QMessageBox::Critical, title, message, QMessageBox::Ok);
  error_box->exec();
}

void ImportFromSteamDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  int row = ui->app_table->currentRow();
  if(row < 0 || row >= ui->app_table->rowCount())
    return;
  QString name = ui->app_table->item(row, 0)->text();
  QString icon_path = ui->app_table->item(row, 0)->data(Qt::UserRole).toString();
  if(!sfs::exists(icon_path.toStdString()))
    icon_path = "";
  QString app_id = ui->app_table->item(row, 1)->text();
  sfs::path path(ui->app_table->item(row, 3)->text().toStdString());
  QString prefix_path = "";
  if(ui->app_table->item(row, 2)->text() == "True")
    prefix_path = (path.parent_path().parent_path() / "compatdata" / app_id.toStdString() / "pfx" /
                   "drive_c")
                    .c_str();
  emit applicationImported(name, app_id, path.string().c_str(), prefix_path, icon_path);
}

void ImportFromSteamDialog::on_path_field_editingFinished()
{
  updateTable(ui->path_field->text().toStdString());
}

void ImportFromSteamDialog::on_search_field_textEdited(const QString& new_text)
{
  if(new_text.isEmpty())
  {
    for(int i = 0; i < ui->app_table->rowCount(); i++)
      ui->app_table->setRowHidden(i, false);
  }
  else
  {
    for(int i = 0; i < ui->app_table->rowCount(); i++)
      ui->app_table->setRowHidden(
        i, !ui->app_table->item(i, 0)->text().contains(new_text, Qt::CaseInsensitive));
  }
}
