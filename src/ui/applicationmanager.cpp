#include "applicationmanager.h"
#include "../core/deployerfactory.h"
#include "../core/installer.h"
#include "../core/pathutils.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <regex>

namespace sfs = std::filesystem;
namespace pu = path_utils;

bool performDownload(ImportModInfo& info, ApplicationManager* app_mgr)
{
  app_mgr->sendLogMessage(Log::LOG_DEBUG,
                          std::format("Downloading from : '{}'", info.remote_download_url));
  std::regex url_regex(R"(.*/(.*)\?.*)");
  std::smatch match;
  if(!std::regex_match(info.remote_download_url, match, url_regex))
    throw std::runtime_error(std::format("Invalid download URL \"{}\"", info.remote_download_url));
  sfs::path download_path = info.target_path;
  if(!sfs::exists(download_path))
    sfs::create_directories(download_path);
  sfs::path file_name = match[1].str();
  const std::string file_name_prefix = file_name.stem();
  const std::string extension = file_name.extension();
  int suffix = 1;
  while(pu::exists(download_path / file_name))
  {
    file_name = file_name_prefix + "(" + std::to_string(suffix) + ")" + extension;
    suffix++;
  }
  std::string file_name_str = file_name.string();
  auto pos = file_name_str.find("%20");
  while(pos != std::string::npos)
  {
    file_name_str.replace(pos, 3, " ");
    pos = file_name_str.find("%20");
  }
  file_name = file_name_str;

  auto progress_callback = [app_mgr](float progress) { app_mgr->sendUpdateProgress(progress); };
  std::ofstream fstream(download_path / file_name, std::ios::binary);
  if(!fstream.is_open())
    throw std::runtime_error("Failed to write to disk.");
  bool message_sent = false;
  cpr::Response response = cpr::Download(
    fstream,
    cpr::Url(info.remote_download_url),
    cpr::ProgressCallback(
      [app_mgr, &message_sent, &file_name, progress_callback](auto download_total,
                                                              auto download_now,
                                                              auto upload_total,
                                                              auto upload_now,
                                                              intptr_t user_data)
      {
        if(!message_sent && download_total > 0)
        {
          std::string size_string;
          long last_size = 0;
          long size = download_total;
          int exp = 0;
          const std::vector<std::string> units{ "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB" };
          while(size > 1024 && exp < units.size())
          {
            last_size = size;
            size /= 1024;
            exp++;
          }
          last_size /= 1.024;
          size_string = std::to_string(size);
          const int first_digit = (last_size / 100) % 10;
          const int second_digit = (last_size / 10) % 10;
          if(first_digit != 0 || second_digit != 0)
            size_string += "." + std::to_string(first_digit);
          if(second_digit != 0)
            size_string += std::to_string(second_digit);
          size_string += units[exp];

          app_mgr->sendLogMessage(
            Log::LOG_INFO,
            ("Downloading \"" + file_name.string() + "\" with size: ").c_str() + size_string +
              "...");
          message_sent = true;
        }
        if(download_total != 0)
          progress_callback((float)download_now / (float)download_total);
        return true;
      }));
  if(response.status_code != 200)
  {
    sfs::remove(download_path / file_name);
    throw std::runtime_error("Download failed with response: \"" + response.status_line +
                             "\" (code " + std::to_string(response.status_code) + ").");
  }
  fstream.close();
  info.local_source = download_path / file_name;
  info.current_path = info.local_source;
  return true;
}

bool performExtraction(ImportModInfo& info, ApplicationManager* app_mgr)
{
  info.last_action_was_successful = false;
  auto progress_callback = [app_mgr](float progress) { app_mgr->sendUpdateProgress(progress); };
  ProgressNode node(progress_callback);
  Installer::extract(info.local_source, info.target_path, &node);
  info.current_path = info.target_path;
  info.last_action_was_successful = true;
  return true;
}

ApplicationManager::ApplicationManager(QObject* parent) : QObject{ parent }
{
  if(number_of_instances_ > 0)
    throw std::runtime_error("Do not instantiate more than one ApplicationManager!");
  number_of_instances_++;
  Installer::log = [app_mgr = this](Log::LogLevel log_level, const std::string& message)
  { app_mgr->sendLogMessage(log_level, message); };
}

ApplicationManager::~ApplicationManager()
{
  number_of_instances_--;
}

void ApplicationManager::init()
{
  updateState();
}

void ApplicationManager::sendLogMessage(Log::LogLevel log_level, const std::string& message)
{
  emit logMessage(log_level, message.c_str());
}

std::string ApplicationManager::toString() const
{
  std::string summary = "";
  for(int i = 0; const auto& app : apps_)
  {
    summary += "[" + std::to_string(i) + "] " + app.name() + "\n";
    for(int j = 0; const auto& profile : app.getProfileNames())
    {
      summary += "\t[" + std::to_string(j) + "] " + profile + "\n";
      j++;
    }
    i++;
  }
  return summary;
}

int ApplicationManager::getNumApplications() const
{
  return apps_.size();
}

int ApplicationManager::getNumProfiles(int app_id) const
{
  if(app_id >= 0 && app_id < apps_.size())
    return apps_[app_id].getProfileNames().size();
  return 0;
}

void ApplicationManager::enableExceptions(bool enabled)
{
  throw_exceptions_ = enabled;
}

void ApplicationManager::updateSettings()
{
  QSettings settings(QCoreApplication::applicationName());
  settings.beginWriteArray("staging_directories");
  for(int i = 0; i < apps_.size(); i++)
  {
    settings.setArrayIndex(i);
    settings.setValue(QString::number(i), apps_[i].getStagingDir().c_str());
  }
  settings.endArray();
}

void ApplicationManager::updateState()
{
  apps_.clear();
  QSettings settings(QCoreApplication::applicationName());
  int num_apps = settings.beginReadArray("staging_directories");
  for(int i = 0; i < num_apps; i++)
  {
    sfs::path staging_dir;
    settings.setArrayIndex(i);
    if(settings.contains(QString::number(i)))
      staging_dir = settings.value(QString::number(i)).toString().toStdString();
    else
    {
      handleParseError(settings.fileName().toStdString(), "Could not parse staging directories.");
      continue;
    }
    int code = ModdedApplication::verifyStagingDir(staging_dir);
    if(code != 0)
    {
      handleAddAppError(code, staging_dir);
      continue;
    }
    try
    {
      apps_.emplace_back(staging_dir);
      apps_.back().setProgressCallback([app_mgr = this](float p)
                                       { app_mgr->sendUpdateProgress(p); });
      apps_.back().setLog([app_mgr = this](Log::LogLevel log_level, const std::string& message)
                          { app_mgr->sendLogMessage(log_level, message); });
    }
    catch(Json::RuntimeError& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
      continue;
    }
    catch(Json::LogicError& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
      continue;
    }
    catch(ParseError& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
      continue;
    }
    catch(sfs::filesystem_error& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
      continue;
    }
    catch(std::runtime_error& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
      continue;
    }
    catch(std::invalid_argument& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
      continue;
    }
    catch(std::logic_error& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
      continue;
    }
    catch(...)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(),
                       "Unexpected error!");
      continue;
    }
  }
  settings.endArray();
}

bool ApplicationManager::appIndexIsValid(int app_id, bool show_error)
{
  if(app_id >= 0 && app_id < apps_.size())
    return true;
  if(show_error)
    emit sendError("Error", "App index \"" + QString::number(app_id) + "\" out of range!");
  return false;
}

bool ApplicationManager::deployerIndexIsValid(int app_id, int deployer, bool show_error)
{
  if(deployer >= 0 && deployer < apps_[app_id].getNumDeployers())
    return true;
  if(show_error)
    emit sendError("Error", "Deployer index \"" + QString::number(deployer) + "\" out of range!");
  return false;
}

void ApplicationManager::handleAddAppError(int code, sfs::path staging_dir)
{
  if(code == 1)
    emit sendError(
      "Error", "Could not read from config file in " + QString(staging_dir.string().c_str()) + "!");
  else
    emit sendError("Error",
                   "Could not parse config file in " + QString(staging_dir.string().c_str()) + "!");
  emit completedOperations();
}

void ApplicationManager::handleAddDeployerError(int code,
                                                sfs::path staging_dir,
                                                sfs::path dest_dir,
                                                const std::string& error_message)
{
  if(code == 1)
    emit sendError("Error",
                   "Could not write to staging dir " + QString(staging_dir.string().c_str()) + "!");
  else if(code == 2)
    emit sendError(
      "Error",
      "Could not create hard link from\n\"" + QString(staging_dir.string().c_str()) + "\"\nto\n\"" +
        QString(dest_dir.string().c_str()) + "\".\n" +
        "Ensure that both directories are on the same partition!\n" "Alternatively: " "Switch to " "sym " "link deployment.");
  else if(code == 3)
    emit sendError("Error",
                   "Could no copy files\n\"" + QString(staging_dir.string().c_str()) +
                     "\"\nto\n\"" + QString(dest_dir.string().c_str()) + "\"!");
  if(code != 0)
    emit logMessage(Log::LOG_ERROR, error_message.c_str());
}

void ApplicationManager::handleParseError(std::string path, std::string message)
{
  emit sendError("Error",
                 ("Error parsing settings file in \"" + path + "\".\n" + message +
                  "\n\n A backup of the last known good state, named \"." +
                  ModdedApplication::CONFIG_FILE_NAME + ".bak\", exists in the same directory.")
                   .c_str());
  emit completedOperations();
}

void ApplicationManager::sendUpdateProgress(float progress)
{
  emit updateProgress(progress);
}

void ApplicationManager::sendLogMessage(Log::LogLevel level, QString message)
{
  emit logMessage(level, message);
}

void ApplicationManager::addApplication(EditApplicationInfo info)
{
  sfs::path staging_dir{ info.staging_dir };
  int code = ModdedApplication::verifyStagingDir(staging_dir);
  if(code == 0)
  {
    try
    {
      apps_.emplace_back(staging_dir, info.name, info.command, info.icon_path, info.app_version);
      apps_.back().setProgressCallback([app_mgr = this](float p)
                                       { app_mgr->sendUpdateProgress(p); });
      apps_.back().setLog([app_mgr = this](Log::LogLevel log_level, const std::string& message)
                          { app_mgr->sendLogMessage(log_level, message); });

      for(const auto& depl_info : info.deployers)
        apps_.back().addDeployer(depl_info);
      apps_.back().fixInvalidHardLinkDeployers();
      for(const auto& tag : info.auto_tags)
        apps_.back().addAutoTag(tag, true);
      updateSettings();
      emit completedOperations("Application added");
    }
    catch(Json::RuntimeError& error)
    {
      handleParseError(staging_dir / ModdedApplication::CONFIG_FILE_NAME, error.what());
    }
    catch(Json::LogicError& error)
    {
      handleParseError(staging_dir / ModdedApplication::CONFIG_FILE_NAME, error.what());
    }
    catch(ParseError& error)
    {
      handleParseError(staging_dir / ModdedApplication::CONFIG_FILE_NAME, error.what());
    }
    catch(sfs::filesystem_error& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
    }
    catch(std::runtime_error& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
    }
    catch(std::invalid_argument& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
    }
    catch(std::logic_error& error)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(), error.what());
    }
    catch(...)
    {
      handleParseError((staging_dir / ModdedApplication::CONFIG_FILE_NAME).string(),
                       "Unexpected error while adding application!");
    }
  }
  else
    handleAddAppError(code, staging_dir);
}

void ApplicationManager::removeApplication(int app_id, bool cleanup)
{
  if(!appIndexIsValid(app_id))
    return;
  if(cleanup)
    handleExceptions<&ModdedApplication::deleteAllData>(app_id);
  apps_.erase(apps_.begin() + app_id);
  updateSettings();
}

void ApplicationManager::deployMods(int app_id)
{
  if(appIndexIsValid(app_id))
  {
    auto ret_val = handleExceptions(&ModdedApplication::verifyDeployerDirectories, apps_[app_id]);
    if(ret_val)
    {
      auto [code, path, message] = *ret_val;
      handleAddDeployerError(code, apps_[app_id].getStagingDir(), path, message);
      if(code == 0)
        handleExceptions<&ModdedApplication::deployMods>(app_id);
    }
  }
  emit completedOperations("Mods deployed");
}

void ApplicationManager::deployModsFor(int app_id, std::vector<int> deployer_ids)
{
  if(appIndexIsValid(app_id))
  {
    for(int deployer : deployer_ids)
    {
      if(!deployerIndexIsValid(app_id, deployer))
      {
        emit completedOperations();
        return;
      }
    }
    auto ret_val = handleExceptions(&ModdedApplication::verifyDeployerDirectories, apps_[app_id]);
    if(ret_val)
    {
      auto [code, path, message] = *ret_val;
      handleAddDeployerError(code, apps_[app_id].getStagingDir(), path, message);
      if(code == 0)
        handleExceptions<&ModdedApplication::deployModsFor>(app_id, deployer_ids);
    }
  }
  emit completedOperations("Mods deployed");
}

void ApplicationManager::unDeployMods(int app_id)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::unDeployMods>(app_id);
  emit completedOperations("Mods undeployed");
}

void ApplicationManager::unDeployModsFor(int app_id, std::vector<int> deployer_ids)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::unDeployModsFor>(app_id, deployer_ids);
  emit completedOperations("Mods undeployed");
}

void ApplicationManager::installMod(int app_id, ImportModInfo info)
{
  bool has_thrown = false;
  if(appIndexIsValid(app_id))
  {
    has_thrown = handleExceptions<&ModdedApplication::installMod>(app_id, info);
    if(has_thrown)
      handleExceptions<&ModdedApplication::cleanupFailedInstallation>(app_id);
  }
  emit modInstallationComplete(!has_thrown);
}

void ApplicationManager::uninstallMods(int app_id,
                                       std::vector<int> mod_ids,
                                       std::string installer_type)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::uninstallMods>(app_id, mod_ids, installer_type);
  emit completedOperations(std::format("Mod{} removed", mod_ids.size() == 1 ? "" : "s").c_str());
}

void ApplicationManager::commitChanges(int app_id, int deployer)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
    handleExceptions<&ModdedApplication::commitChanges>(app_id);
}

void ApplicationManager::updateModDeployers(int app_id,
                                            std::vector<int> mod_ids,
                                            std::vector<bool> deployers)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::updateModDeployers>(app_id, mod_ids, deployers);
  emit completedOperations("Deployers updated");
}

void ApplicationManager::removeNodeFromDeployer(int app_id, int deployer, void *node_ptr)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
    handleExceptions<&ModdedApplication::removeNodeFromDeployer>(
      app_id, deployer, node_ptr, true, std::optional<ProgressNode*>{});
  emit completedOperations("Deployers updated");
}

void ApplicationManager::setModStatus(int app_id, int deployer, int mod_id, bool status)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
    handleExceptions<&ModdedApplication::setModStatus>(app_id, deployer, mod_id, status);
}

void ApplicationManager::addDeployer(int app_id, EditDeployerInfo info)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::addDeployer>(app_id, info);
  emit completedOperations("Deployer added");
}

void ApplicationManager::removeDeployer(int app_id, int deployer, bool cleanup)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
    handleExceptions<&ModdedApplication::removeDeployer>(app_id, deployer, cleanup);
}

void ApplicationManager::getDeployerNames(int app_id, bool is_new)
{
  QStringList names{};
  if(appIndexIsValid(app_id, false))
  {
    for(const auto& name : apps_[app_id].getDeployerNames())
      names << name.c_str();
  }
  emit sendDeployerNames(names, is_new);
}

void ApplicationManager::getModInfo(int app_id)
{
  if(appIndexIsValid(app_id, false))
  {
    auto info = handleExceptions(&ModdedApplication::getModInfo, apps_[app_id]);
    if(info)
    {
      emit sendModInfo(*info);
      return;
    }
  }
  emit sendModInfo(std::vector<ModInfo>{});
}

void ApplicationManager::getDeployerInfo(int app_id, int deployer)
{
  if(appIndexIsValid(app_id, false) && deployerIndexIsValid(app_id, deployer, false))
  {
    auto info = handleExceptions(&ModdedApplication::getDeployerInfo, apps_[app_id], deployer);
    if(info)
    {
      emit sendDeployerInfo(*info);
      return;
    }
  }
  emit sendDeployerInfo(DeployerInfo{});
}

void ApplicationManager::getApplicationNames(bool is_new)
{
  QStringList names{};
  QStringList icon_paths{};
  for(const auto& app : apps_)
  {
    names.append(app.name().c_str());
    icon_paths.append(app.iconPath().string().c_str());
  }
  emit sendApplicationNames(names, icon_paths, is_new);
}

void ApplicationManager::changeModName(int app_id, int mod_id, QString new_name)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::changeModName>(app_id, mod_id, new_name.toStdString());
}

void ApplicationManager::getFileConflicts(int app_id, int deployer, int mod_id, bool show_disabled)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
  {
    auto conflicts = handleExceptions(
      &ModdedApplication::getFileConflicts, apps_[app_id], deployer, mod_id, show_disabled);
    if(conflicts)
      emit sendFileConflicts(*conflicts);
  }
  emit completedOperations();
}

void ApplicationManager::getAppInfo(int app_id)
{
  if(!appIndexIsValid(app_id, false))
  {
    emit sendAppInfo(AppInfo{});
    return;
  }
  emit sendAppInfo(apps_[app_id].getAppInfo());
}

void ApplicationManager::addTool(int app_id, Tool tool)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::addTool>(app_id, tool);
}

void ApplicationManager::removeTool(int app_id, int tool_id)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::removeTool>(app_id, tool_id);
}

void ApplicationManager::editApplication(EditApplicationInfo info, int app_id)
{
  if(appIndexIsValid(app_id))
  {
    if(!handleExceptions<&ModdedApplication::setStagingDir>(
         app_id, info.staging_dir, info.move_staging_dir))
    {
      handleExceptions<&ModdedApplication::setName>(app_id, info.name);
      handleExceptions<&ModdedApplication::setCommand>(app_id, info.command);
      handleExceptions<&ModdedApplication::setIconPath>(app_id, info.icon_path);
      handleExceptions<&ModdedApplication::setAppVersion>(app_id, info.app_version);
    }
    updateSettings();
  }
}

void ApplicationManager::editDeployer(EditDeployerInfo info, int app_id, int deployer)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
    handleExceptions<&ModdedApplication::editDeployer>(app_id, deployer, info);
}

void ApplicationManager::getModConflicts(int app_id, int deployer, int mod_id)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
  {
    auto conflicts =
      handleExceptions(&ModdedApplication::getModConflicts, apps_[app_id], deployer, mod_id);
    if(conflicts)
      emit sendModConflicts(*conflicts);
  }
  emit completedOperations();
}

void ApplicationManager::setProfile(int app_id, int profile)
{
  if(appIndexIsValid(app_id, false))
    handleExceptions<&ModdedApplication::setProfile>(app_id, profile);
}

void ApplicationManager::addProfile(int app_id, EditProfileInfo info)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::addProfile>(app_id, info);
}

void ApplicationManager::removeProfile(int app_id, int profile)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::removeProfile>(app_id, profile);
}

void ApplicationManager::getProfileNames(int app_id, bool is_new)
{
  QStringList names{};
  if(appIndexIsValid(app_id, false))
  {
    for(const auto& name : apps_[app_id].getProfileNames())
      names << name.c_str();
  }
  emit sendProfileNames(names, is_new);
}

void ApplicationManager::editProfile(int app_id, int profile, EditProfileInfo info)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::editProfile>(app_id, profile, info);
}

void ApplicationManager::editTool(int app_id, int tool_id, Tool new_tool)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::editTool>(app_id, tool_id, new_tool);
}

void ApplicationManager::addModToGroup(int app_id, int mod_id, int group)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::addModToGroup>(
      app_id, mod_id, group, std::optional<ProgressNode*>{});
  emit completedOperations("Mod added to group");
}

void ApplicationManager::removeModFromGroup(int app_id, int mod_id)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::removeModFromGroup>(
      app_id, mod_id, true, std::optional<ProgressNode*>{});
  emit completedOperations("Mod removed from group");
}

void ApplicationManager::createGroup(int app_id, int first_mod_id, int second_mod_id)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::createGroup>(
      app_id, first_mod_id, second_mod_id, std::optional<ProgressNode*>{});
  emit completedOperations("Mod added to group");
}

void ApplicationManager::changeActiveGroupMember(int app_id, int group, int mod_id)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::changeActiveGroupMember>(
      app_id, group, mod_id, std::optional<ProgressNode*>{});
  emit completedOperations();
}

void ApplicationManager::changeModVersion(int app_id, int mod_id, QString new_version)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::changeModVersion>(
      app_id, mod_id, new_version.toStdString());
}

void ApplicationManager::sortModsByConflicts(int app_id, int deployer)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
    handleExceptions<&ModdedApplication::sortModsByConflicts>(app_id, deployer);
  emit completedOperations("Mods sorted");
}

void ApplicationManager::extractArchive(ImportModInfo info)
{
  handleExceptionsForFunction(performExtraction, info, this);
  emit extractionComplete(info);
}

void ApplicationManager::addBackupTarget(int app_id,
                                         QString path,
                                         QString name,
                                         QString default_backup,
                                         QString first_backup)
{
  if(appIndexIsValid(app_id))
  {
    std::vector<std::string> backup_names{ default_backup.toStdString() };
    if(!first_backup.isEmpty())
      backup_names.push_back(first_backup.toStdString());
    handleExceptions<&ModdedApplication::addBackupTarget>(
      app_id, path.toStdString(), name.toStdString(), backup_names);
  }
  emit completedOperations("Backup target added");
}

void ApplicationManager::removeBackupTarget(int app_id, int target_id)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::removeBackupTarget>(app_id, target_id);
  emit completedOperations("Backup target removed");
}

void ApplicationManager::addBackup(int app_id, int target_id, QString name, int source)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::addBackup>(app_id, target_id, name.toStdString(), source);
  emit completedOperations("Backup added");
}

void ApplicationManager::removeBackup(int app_id, int target_id, int backup_id)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::removeBackup>(app_id, target_id, backup_id);
  emit completedOperations("Backup removed");
}

void ApplicationManager::setActiveBackup(int app_id, int target_id, int backup_id)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::setActiveBackup>(app_id, target_id, backup_id);
  emit completedOperations();
}

void ApplicationManager::getBackupTargets(int app_id)
{
  if(appIndexIsValid(app_id))
    emit sendBackupTargets(apps_[app_id].getBackupTargets());
}

void ApplicationManager::setBackupName(int app_id, int target_id, int backup_id, QString name)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::setBackupName>(
      app_id, target_id, backup_id, name.toStdString());
}

void ApplicationManager::setBackupTargetName(int app_id, int target_id, QString name)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::setBackupTargetName>(
      app_id, target_id, name.toStdString());
}

void ApplicationManager::overwriteBackup(int app_id,
                                         int target_id,
                                         int source_backup,
                                         int dest_backup)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::overwriteBackup>(
      app_id, target_id, source_backup, dest_backup);
  emit completedOperations("Backup overwritten");
}

void ApplicationManager::onScrollLists()
{
  emit scrollLists();
}

void ApplicationManager::uninstallGroupMembers(int app_id, const std::vector<int>& mod_ids)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::uninstallGroupMembers>(app_id, mod_ids);
  emit completedOperations("Group members removed");
}

void ApplicationManager::addManualTag(int app_id, QString tag_name)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::addManualTag>(app_id, tag_name.toStdString());
}

void ApplicationManager::removeManualTag(int app_id, QString tag_name)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::removeManualTag>(app_id, tag_name.toStdString(), true);
}

void ApplicationManager::changeManualTagName(int app_id, QString old_name, QString new_name)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::changeManualTagName>(
      app_id, old_name.toStdString(), new_name.toStdString(), true);
}

void ApplicationManager::addTagsToMods(int app_id,
                                       QStringList tag_names,
                                       const std::vector<int>& mod_ids)
{
  if(!appIndexIsValid(app_id))
    return;

  std::vector<std::string> tag_vector;
  for(const auto& tag_name : tag_names)
    tag_vector.push_back(tag_name.toStdString());
  handleExceptions<&ModdedApplication::addTagsToMods>(app_id, tag_vector, mod_ids);
}

void ApplicationManager::removeTagsFromMods(int app_id,
                                            QStringList tag_names,
                                            const std::vector<int>& mod_ids)
{
  if(!appIndexIsValid(app_id))
    return;

  std::vector<std::string> tag_vector;
  for(const auto& tag_name : tag_names)
    tag_vector.push_back(tag_name.toStdString());
  handleExceptions<&ModdedApplication::removeTagsFromMods>(app_id, tag_vector, mod_ids);
}

void ApplicationManager::setTagsForMods(int app_id,
                                        QStringList tag_names,
                                        const std::vector<int>& mod_ids)
{
  if(appIndexIsValid(app_id))
  {
    std::vector<std::string> string_vec;
    for(const auto& name : tag_names)
      string_vec.push_back(name.toStdString());
    handleExceptions<&ModdedApplication::setTagsForMods>(app_id, string_vec, mod_ids);
  }
}

void ApplicationManager::editManualTags(int app_id, std::vector<EditManualTagAction> actions)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::editManualTags>(app_id, actions);
}

void ApplicationManager::editAutoTags(int app_id, std::vector<EditAutoTagAction> actions)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::editAutoTags>(app_id, actions);
  emit completedOperations("Auto tags updated");
}

void ApplicationManager::reapplyAutoTags(int app_id)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::reapplyAutoTags>(app_id);
  emit completedOperations("Auto tags updated");
}

void ApplicationManager::updateAutoTags(int app_id, std::vector<int> mod_ids)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::updateAutoTags>(app_id, mod_ids);
  emit completedOperations("Auto tags updated");
}

void ApplicationManager::editModSources(int app_id,
                                        int mod_id,
                                        QString local_source,
                                        QString remote_source)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::setModSources>(
      app_id, mod_id, local_source.toStdString(), remote_source.toStdString());
}

void ApplicationManager::getNexusPage(int app_id, int mod_id)
{
  if(appIndexIsValid(app_id))
  {
    auto page = handleExceptions(&ModdedApplication::getNexusPage, apps_[app_id], mod_id);
    if(page)
      emit sendNexusPage(app_id, mod_id, *page);
  }
  emit completedOperations();
}

void ApplicationManager::downloadMod(ImportModInfo info)
{
  info.last_action_was_successful = false;
  if(!appIndexIsValid(info.app_id))
  {
    emit downloadFailed();
    return;
  }

  if(info.remote_request_url.empty())
  {
    auto download_url = handleExceptionsForFunction(
      static_cast<std::string (*)(const std::string&, long)>(nexus::Api::getDownloadUrl),
      info.remote_source,
      info.remote_file_id);
    if(!download_url)
    {
      emit downloadFailed();
      return;
    }
    info.remote_download_url = *download_url;
  }
  else
  {
    auto download_url = handleExceptionsForFunction(
      static_cast<std::string (*)(const std::string&)>(nexus::Api::getDownloadUrl),
      info.remote_request_url);
    if(!download_url)
    {
      emit downloadFailed();
      return;
    }
    info.remote_download_url = *download_url;
  }
  info.remote_download_url = QUrl(info.remote_download_url.c_str()).toEncoded().toStdString();

  auto init_successful = handleExceptionsForFunction(nexus::Api::initModInfo, info);
  if(!init_successful || !(*init_successful))
  {
    emit downloadFailed();
    return;
  }

  info.target_path = apps_[info.app_id].getDownloadDir();
  auto download_successful = handleExceptionsForFunction(performDownload, info, this);
  if(!download_successful)
  {
    emit downloadFailed();
    return;
  }

  info.last_action_was_successful = true;
  emit downloadComplete(info);
}

void ApplicationManager::checkForModUpdates(int app_id)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::checkForModUpdates>(app_id);
  emit completedOperations();
}

void ApplicationManager::checkModsForUpdates(int app_id, const std::vector<int>& mod_ids)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::checkModsForUpdates>(app_id, mod_ids);
  emit completedOperations();
}

void ApplicationManager::suppressUpdateNotification(int app_id, const std::vector<int>& mod_ids)
{
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::suppressUpdateNotification>(app_id, mod_ids);
  emit completedOperations();
}

void ApplicationManager::getExternalChanges(int app_id, int deployer, bool deploy)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
  {
    auto changes_info =
      handleExceptions(&ModdedApplication::getExternalChanges, apps_[app_id], deployer);
    if(!changes_info)
      emit completedOperations("Checking for external changes failed");
    else
      emit sendExternalChangesInfo(app_id, *changes_info, apps_[app_id].getNumDeployers(), deploy);
  }
  else
    emit completedOperations("Checking for external changes failed");
}

void ApplicationManager::keepOrRevertFileModifications(int app_id,
                                                       int deployer,
                                                       const FileChangeChoices& changes_to_keep,
                                                       bool deploy)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
  {
    const bool has_throw = handleExceptions<&ModdedApplication::keepOrRevertFileModifications>(
      app_id, deployer, changes_to_keep);
    if(has_throw)
      emit completedOperations("Applying external changes failed");
    else
      emit externalChangesHandled(app_id, deployer, apps_[app_id].getNumDeployers(), deploy);
  }
  else
    emit completedOperations("Applying external changes failed");
}

void ApplicationManager::exportAppConfiguration(int app_id,
                                                std::vector<int> deployers,
                                                QStringList auto_tags)
{
  std::vector<std::string> tag_vector;
  for(const auto& tag : auto_tags)
    tag_vector.push_back(tag.toStdString());
  if(appIndexIsValid(app_id))
    handleExceptions<&ModdedApplication::exportConfiguration>(app_id, deployers, tag_vector);
  emit completedOperations("Configuration exported");
}

void ApplicationManager::updateIgnoredFiles(int app_id, int deployer)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
    handleExceptions<&ModdedApplication::updateIgnoredFiles>(app_id, deployer);
  emit completedOperations("Ignore list updated");
}

void ApplicationManager::addModToIgnoreList(int app_id, int deployer, int mod_id)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
    handleExceptions<&ModdedApplication::addModToIgnoreList>(app_id, deployer, mod_id);
}

void ApplicationManager::applyModAction(int app_id, int deployer, int action, int mod_id)
{
  if(appIndexIsValid(app_id) && deployerIndexIsValid(app_id, deployer))
    handleExceptions<&ModdedApplication::applyModAction>(app_id, deployer, action, mod_id);
}
