#include "settingsdialog.h"
#include "../core/log.h"
#include "../core/lootdeployer.h"
#include "../core/nexus/api.h"
#include "addapikeydialog.h"
#include "changeapipwdialog.h"
#include "core/cryptography.h"
#include "core/installer.h"
#include "core/parseerror.h"
#include "enterapipwdialog.h"
#include "ui_settingsdialog.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <ranges>

namespace str = std::ranges;


SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), ui(new Ui::SettingsDialog)
{
  ui->setupUi(this);
  ui->show_api_key_button->setIcon(show_icon);
  ui->show_api_key_button->setToolTip("Show API Key");
  ui->api_key_label->setText(api_key_hidden_string);
}

SettingsDialog::~SettingsDialog()
{
  delete ui;
}

void SettingsDialog::init()
{
  QSettings settings(QCoreApplication::applicationName());
  ui->remove_mod_cb->setCheckState(settings.value("ask_remove_mod", true).toBool() ? Qt::Checked
                                                                                   : Qt::Unchecked);
  ui->remove_from_dep_cb->setCheckState(
    settings.value("ask_remove_from_deployer", true).toBool() ? Qt::Checked : Qt::Unchecked);
  ui->remove_prof_cb->setCheckState(
    settings.value("ask_remove_profile", true).toBool() ? Qt::Checked : Qt::Unchecked);
  ui->remove_bak_target_cb->setCheckState(
    settings.value("ask_remove_backup_target", true).toBool() ? Qt::Checked : Qt::Unchecked);
  ui->remove_bak_cb->setCheckState(
    settings.value("ask_remove_backup", true).toBool() ? Qt::Checked : Qt::Unchecked);
  ui->log_level_box->setCurrentIndex(settings.value("log_level", Log::LogLevel::LOG_INFO).toInt() -
                                     2);
  ui->fo3_url_field->setText(
    settings.value("fo3_url", LootDeployer::DEFAULT_LIST_URLS.at(loot::GameType::fo3).c_str())
      .toString());
  ui->fo4_url_field->setText(
    settings.value("fo4_url", LootDeployer::DEFAULT_LIST_URLS.at(loot::GameType::fo4).c_str())
      .toString());
  ui->fo4vr_url_field->setText(
    settings.value("fo4vr_url", LootDeployer::DEFAULT_LIST_URLS.at(loot::GameType::fo4vr).c_str())
      .toString());
  ui->fonv_url_field->setText(
    settings.value("fonv_url", LootDeployer::DEFAULT_LIST_URLS.at(loot::GameType::fonv).c_str())
      .toString());
  ui->starfield_url_field->setText(
    settings
      .value("starfield_url", LootDeployer::DEFAULT_LIST_URLS.at(loot::GameType::starfield).c_str())
      .toString());
  ui->tes3_url_field->setText(
    settings.value("tes3_url", LootDeployer::DEFAULT_LIST_URLS.at(loot::GameType::tes3).c_str())
      .toString());
  ui->tes4_url_field->setText(
    settings.value("tes4_url", LootDeployer::DEFAULT_LIST_URLS.at(loot::GameType::tes4).c_str())
      .toString());
  ui->tes5_url_field->setText(
    settings.value("tes5_url", LootDeployer::DEFAULT_LIST_URLS.at(loot::GameType::tes5).c_str())
      .toString());
  ui->tes5se_url_field->setText(
    settings.value("tes5se_url", LootDeployer::DEFAULT_LIST_URLS.at(loot::GameType::tes5se).c_str())
      .toString());
  ui->tes5vr_url_field->setText(
    settings.value("tes5vr_url", LootDeployer::DEFAULT_LIST_URLS.at(loot::GameType::tes5vr).c_str())
      .toString());
  ui->show_warning_cb->setCheckState(
    settings.value("log_on_warning", true).toBool() ? Qt::Checked : Qt::Unchecked);
  ui->show_error_cb->setCheckState(settings.value("log_on_error", true).toBool() ? Qt::Checked
                                                                                  : Qt::Unchecked);
  ui->deploy_for_box->setCurrentIndex(settings.value("deploy_for_all", true).toBool() ? 0 : 1);
  ui->unrar_path_field->setText(Installer::UNRAR_PATH.c_str());

  settings.beginGroup("nexus");
  ui->premium_user_label->setText(
    settings.value("info_is_premium").toBool() ? "Account is premium" : "Account is NOT premium");
  const bool key_is_valid = settings.value("info_is_valid").toBool();
  ui->valid_api_key_label->setText(key_is_valid ? "Nexus API key is valid."
                                                : "No valid API key set");
  ui->api_key_label->setVisible(key_is_valid);
  ui->show_api_key_button->setVisible(key_is_valid);
  const int cipher_size = settings.beginReadArray("info_c");
  settings.endArray();
  const int nonce_size = settings.beginReadArray("info_n");
  settings.endArray();
  const int tag_size = settings.beginReadArray("info_t");
  settings.endArray();
  if(cipher_size == 0 || nonce_size == 0 || tag_size == 0)
    ui->change_api_pw_button->setVisible(false);
  else
    ui->change_api_pw_button->setVisible(true);
  settings.endGroup();
  dialog_completed_ = false;
}

void SettingsDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  QSettings settings(QCoreApplication::applicationName());

  settings.setValue("fo3_url", ui->fo3_url_field->text());
  LootDeployer::LIST_URLS[loot::GameType::fo3] = ui->fo3_url_field->text().toStdString();
  settings.setValue("fo4_url", ui->fo4_url_field->text());
  LootDeployer::LIST_URLS[loot::GameType::fo4] = ui->fo4_url_field->text().toStdString();
  settings.setValue("fo4vr_url", ui->fo4vr_url_field->text());
  LootDeployer::LIST_URLS[loot::GameType::fo4vr] = ui->fo4vr_url_field->text().toStdString();
  settings.setValue("fonv_url", ui->fonv_url_field->text());
  LootDeployer::LIST_URLS[loot::GameType::fonv] = ui->fonv_url_field->text().toStdString();
  settings.setValue("starfield_url", ui->starfield_url_field->text());
  LootDeployer::LIST_URLS[loot::GameType::starfield] =
    ui->starfield_url_field->text().toStdString();
  settings.setValue("tes3_url", ui->tes3_url_field->text());
  LootDeployer::LIST_URLS[loot::GameType::tes3] = ui->tes3_url_field->text().toStdString();
  settings.setValue("tes4_url", ui->tes4_url_field->text());
  LootDeployer::LIST_URLS[loot::GameType::tes4] = ui->tes4_url_field->text().toStdString();
  settings.setValue("tes5_url", ui->tes5_url_field->text());
  LootDeployer::LIST_URLS[loot::GameType::tes5] = ui->tes5_url_field->text().toStdString();
  settings.setValue("tes5se_url", ui->tes5se_url_field->text());
  LootDeployer::LIST_URLS[loot::GameType::tes5se] = ui->tes5se_url_field->text().toStdString();
  settings.setValue("tes5vr_url", ui->tes5vr_url_field->text());
  LootDeployer::LIST_URLS[loot::GameType::tes5vr] = ui->tes5vr_url_field->text().toStdString();

  Log::log_level = static_cast<Log::LogLevel>(ui->log_level_box->currentIndex() + 2);
  settings.setValue("log_level", Log::log_level);

  deploy_all_ = ui->deploy_for_box->currentIndex() == 0;
  settings.setValue("deploy_for_all", deploy_all_);

  log_on_error_ = ui->show_error_cb->isChecked();
  settings.setValue("log_on_error", log_on_error_);

  log_on_warning_ = ui->show_warning_cb->isChecked();
  settings.setValue("log_on_warning", log_on_warning_);

  ask_remove_from_deployer_ = ui->remove_from_dep_cb->isChecked();
  settings.setValue("ask_remove_from_deployer", ask_remove_from_deployer_);

  ask_remove_mod_ = ui->remove_mod_cb->isChecked();
  settings.setValue("ask_remove_mod", ask_remove_mod_);

  ask_remove_profile_ = ui->remove_prof_cb->isChecked();
  settings.setValue("ask_remove_profile", ask_remove_profile_);

  ask_remove_backup_target_ = ui->remove_bak_target_cb->isChecked();
  settings.setValue("ask_remove_backup_target", ask_remove_backup_target_);

  ask_remove_backup_ = ui->remove_bak_cb->isChecked();
  settings.setValue("ask_remove_backup", ask_remove_backup_);

  Installer::UNRAR_PATH = ui->unrar_path_field->text().toStdString();
  settings.setValue("unrar_path", ui->unrar_path_field->text());

  emit settingsDialogAccepted();
}

bool SettingsDialog::askRemoveBackup() const
{
  return ask_remove_backup_;
}

std::optional<std::tuple<std::string, std::string, std::string, bool>>
SettingsDialog::getNexusApiKeyDetails()
{
  QSettings settings(QCoreApplication::applicationName());
  settings.beginGroup("nexus");
  const int cipher_size = settings.beginReadArray("info_c");
  settings.endArray();
  const int nonce_size = settings.beginReadArray("info_n");
  settings.endArray();
  const int tag_size = settings.beginReadArray("info_t");
  settings.endArray();
  if(cipher_size == 0 || nonce_size == 0 || tag_size == 0)
    return {};

  int array_size = settings.beginReadArray("info_c");
  std::string cipher;
  cipher.reserve(array_size);
  for(int i = 0; i < array_size; i++)
  {
    settings.setArrayIndex(i);
    cipher.append({ static_cast<char>(settings.value("uchar").toUInt()) });
  }
  settings.endArray();

  array_size = settings.beginReadArray("info_n");
  std::string nonce;
  nonce.reserve(array_size);
  for(int i = 0; i < array_size; i++)
  {
    settings.setArrayIndex(i);
    nonce.append({ static_cast<char>(settings.value("uchar").toUInt()) });
  }
  settings.endArray();

  array_size = settings.beginReadArray("info_t");
  std::string tag;
  tag.reserve(array_size);
  for(int i = 0; i < array_size; i++)
  {
    settings.setArrayIndex(i);
    tag.append({ static_cast<char>(settings.value("uchar").toUInt()) });
  }
  settings.endArray();

  const bool is_default_pw = settings.value("info_is_default").toBool();
  settings.endGroup();
  return { { cipher, nonce, tag, is_default_pw } };
}

void SettingsDialog::setNexusCryptographyFields(const std::string& cipher,
                                                const std::string& nonce,
                                                const std::string& tag,
                                                bool uses_default_pw)
{
  QSettings settings(QCoreApplication::applicationName());
  settings.beginGroup("nexus");
  settings.beginWriteArray("info_c", cipher.size());
  for(auto [i, c] : str::enumerate_view(cipher))
  {
    settings.setArrayIndex(i);
    settings.setValue("uchar", static_cast<unsigned char>(c));
  }
  settings.endArray();
  settings.beginWriteArray("info_n", nonce.size());
  for(auto [i, c] : str::enumerate_view(nonce))
  {
    settings.setArrayIndex(i);
    settings.setValue("uchar", static_cast<unsigned char>(c));
  }
  settings.endArray();
  settings.beginWriteArray("info_t", tag.size());
  for(auto [i, c] : str::enumerate_view(tag))
  {
    settings.setArrayIndex(i);
    settings.setValue("uchar", static_cast<unsigned char>(c));
  }
  settings.endArray();
  settings.setValue("info_is_default", uses_default_pw);
  settings.endGroup();
}

bool SettingsDialog::askRemoveBackupTarget() const
{
  return ask_remove_backup_target_;
}

bool SettingsDialog::logOnWarning() const
{
  return log_on_warning_;
}

bool SettingsDialog::logOnError() const
{
  return log_on_error_;
}

bool SettingsDialog::deployAll() const
{
  return deploy_all_;
}

bool SettingsDialog::askRemoveProfile() const
{
  return ask_remove_profile_;
}

bool SettingsDialog::askRemoveFromDeployer() const
{
  return ask_remove_from_deployer_;
}

bool SettingsDialog::askRemoveMod() const
{
  return ask_remove_mod_;
}

void SettingsDialog::on_unrar_path_button_clicked()
{
  QString starting_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  QString current_path = ui->unrar_path_field->text();
  if(!current_path.isEmpty() && std::filesystem::exists(current_path.toStdString()))
    starting_dir = std::filesystem::path(current_path.toStdString()).parent_path().c_str();
  auto dialog = new QFileDialog;
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setWindowTitle("Select unrar Executable");
  connect(dialog, &QFileDialog::fileSelected, this, &SettingsDialog::onUnrarPathSelected);
  dialog->exec();
}

void SettingsDialog::onUnrarPathSelected(const QString& path)
{
  ui->unrar_path_field->setText(path);
}

void SettingsDialog::on_set_api_key_button_clicked()
{
  AddApiKeyDialog dialog;
  if(dialog.exec() == QDialog::Rejected)
    return;
  std::optional<std::pair<std::string, int>> user_info{};
  auto handle_error = [](auto& e)
  {
    const QString message = "Error while parsing the Answer from NexusMods: \n" + QString(e.what());
    Log::error(message.toStdString());
    QMessageBox error_box(QMessageBox::Critical, "Network Error", message, QMessageBox::Ok);
    error_box.exec();
  };
  const std::string api_key = dialog.getApiKey().toStdString();
  try
  {
    user_info = nexus::Api::validateKey(api_key);
  }
  catch(ParseError& e)
  {
    handle_error(e);
  }
  catch(Json::RuntimeError& e)
  {
    handle_error(e);
  }
  catch(Json::LogicError& e)
  {
    handle_error(e);
  }
  if(!user_info)
  {
    const QString message = "Api key failed to validate.";
    Log::error(message.toStdString());
    QMessageBox error_box(QMessageBox::Critical, "Error", message, QMessageBox::Ok);
    error_box.exec();
    return;
  }
  std::string password =
    dialog.getPassword().isEmpty() ? cryptography::default_key : dialog.getPassword().toStdString();

  std::tuple<std::string, std::string, std::string> res;
  try
  {
    res = cryptography::encrypt(api_key, password);
  }
  catch(CryptographyError& e)
  {
    const QString message = "Error during key encryption";
    Log::error(message.toStdString());
    QMessageBox error_box(QMessageBox::Critical, "Error", message, QMessageBox::Ok);
    error_box.exec();
    return;
  }
  ui->valid_api_key_label->setText(
    ("Valid API key for user: \"" + user_info->first + "\"").c_str());
  ui->premium_user_label->setText(user_info->second ? "Account is premium"
                                                    : "Account is NOT premium");
  nexus::Api::setApiKey(api_key);
  const auto& [cipher, nonce, tag] = res;
  setNexusCryptographyFields(cipher, nonce, tag, dialog.getPassword().isEmpty());
  QSettings settings(QCoreApplication::applicationName());
  settings.beginGroup("nexus");
  settings.setValue("info_is_valid", true);
  settings.setValue("info_is_premium", user_info->second);
  settings.endGroup();
}

void SettingsDialog::on_change_api_pw_button_clicked()
{
  auto res = getNexusApiKeyDetails();
  if(!res)
    return;
  const auto [cipher, nonce, tag, is_default_pw] = *res;
  auto dialog = ChangeApiPwDialog(is_default_pw, cipher, nonce, tag, this);
  connect(&dialog,
          &ChangeApiPwDialog::keyEncryptionUpdated,
          this,
          &SettingsDialog::setNexusCryptographyFields);
  dialog.exec();
}

void SettingsDialog::on_show_api_key_button_clicked()
{
  const bool is_hidden = ui->api_key_label->text() == api_key_hidden_string;
  if(!is_hidden)
  {
    ui->show_api_key_button->setIcon(show_icon);
    ui->show_api_key_button->setToolTip("Show API Key");
    ui->api_key_label->setText(api_key_hidden_string);
    return;
  }
  std::string api_key;
  if(nexus::Api::isInitialized())
    api_key = nexus::Api::getApiKey();
  else
  {
    auto res = getNexusApiKeyDetails();
    if(!res)
    {
      const QString message = "Could not find an API key. Please enter one in the settings dialog.";
      Log::error(message.toStdString());
      QMessageBox error_box(QMessageBox::Critical, "Error", message, QMessageBox::Ok);
      error_box.exec();
      return;
    }
    const auto [cipher, nonce, tag, is_default_pw] = *res;
    std::string pw = cryptography::default_key;
    if(!is_default_pw)
    {
      EnterApiPwDialog dialog(cipher, nonce, tag, this);
      dialog.exec();
      if(!dialog.wasSuccessful())
        return;
      api_key = dialog.getApiKey();
      nexus::Api::setApiKey(api_key);
    }
  }
  ui->api_key_label->setText(("API Key: " + api_key).c_str());
  ui->show_api_key_button->setIcon(hide_icon);
  ui->show_api_key_button->setToolTip("Hide API Key");
}
