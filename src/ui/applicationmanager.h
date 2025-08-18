/*!
 * \file applicationmanager.h
 * \brief Header for the ApplicationManager class.
 */

#pragma once

#include "../core/compressionerror.h"
#include "../core/editapplicationinfo.h"
#include "../core/editautotagaction.h"
#include "../core/editmanualtagaction.h"
#include "../core/log.h"
#include "../core/moddedapplication.h"
#include "../core/nexus/api.h"
#include "../core/parseerror.h"
#include <QDebug>
#include <QObject>
#include <QStandardPaths>
#include <filesystem>


/*!
 * \brief Contains several ModdedApplication objects and provides access to their functions
 * using Qt's signal/ slot mechanism.
 *
 * This is intended to be run inside of a worker thread, therefore public functions are
 * implemented as Qt slots and emit Qt signals instead of returning a value directly.
 * The internal state of this object is stored in a JSON file in the user directory,
 * usually in "~/.local/share/linux_mod_manager/lmm_apps.json".
 * Warning: To ensure all actions are completed as intended, use Qt::QueuedConnection as type
 * for all connections. Do not instantiate more than one object of this class.
 */
class ApplicationManager : public QObject
{
  Q_OBJECT
public:
  /*!
   * \brief Constructor. Only one instance of this class is support at a time.
   * \param parent This is passed to the constructor of QObject.
   * \throws std::runtime_error Indicates that another instance of this class exists.
   */
  explicit ApplicationManager(QObject* parent = nullptr);
  /*! \brief Decreases the static number of instances counter. */
  virtual ~ApplicationManager();

  /*!
   * \brief If a JSON file with settings already exists for this user: Restores
   * the internal state from that file. Else: Creates a new settings file.
   */
  void init();
  /*!
   * \brief Sends a log message to the logging window.
   * \param log_level Type of message.
   * \param message Message to be displayed.
   */
  void sendLogMessage(Log::LogLevel log_level, const std::string& message);
  /*!
   *  \brief Generates a string which contains the ids and names of every application
   *  as well as their profiles.
   */
  std::string toString() const;
  /*! \brief Returns the number of managed \ModdedApplication "applications". */
  int getNumApplications() const;
  /*!
   * \brief Returns the number of profiles for one application.
   * \param app_id Application for which to get the number of profiles.
   * \return The number.
   */
  int getNumProfiles(int app_id) const;
  /*!
   * \brief Enable or disable throwing exceptions.
   * \param enabled New status.
   */
  void enableExceptions(bool enabled);
  /*!
   * \brief Informs about the progress in the current task by emitting \ref updateProgress.
   * \param progress The progress.
   */
  void sendUpdateProgress(float progress);

private:
  /*!
   * \brief Wrapper for member functions of ModdedApplication. Calls the function specified
   * in the template for the ModdedApplication stored at apps_[app_id] with the given
   * arguments and handles all exceptions thrown by the target function.
   * \param app_id Target app id.
   * \param args Arguments which are passed to the function.
   * \return True iff an exception has been thrown.
   */
  template<auto f, typename... Args>
  bool handleExceptions(int app_id, Args&&... args)
  {
    std::string message;
    bool has_thrown = false;
    try
    {
      (this->apps_[app_id].*f)(std::forward<Args>(args)...);
    }
    catch(Json::RuntimeError& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(Json::LogicError& error)
    {
      has_thrown = true;

      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(ParseError& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(std::ios_base::failure& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(CompressionError& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(std::runtime_error& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(std::invalid_argument& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(...)
    {
      has_thrown = true;
      message = "An unexpected error occured!";
      if(throw_exceptions_)
        throw std::runtime_error("An unexpected error occured!");
    }

    if(has_thrown)
      emit sendError("Error", message.c_str());
    return has_thrown;
  }

  /*!
   * \brief Wrapper for class member functions. Catches specific exception types and sends an error
   * message to the gui if an exception was thrown.
   * \param f Function to run.
   * \param obj Object of a class that contains f as member function.
   * \param args Arguments for the function.
   * \return If no exception was thrown: The return value of the function,
   * else: An empty optional.
   */
  template<typename Func, typename Obj, typename... Args>
  auto handleExceptions(Func&& f, Obj&& obj, Args&&... args)
    -> std::optional<decltype((obj.*f)(std::forward<Args>(args)...))>
  {
    decltype((obj.*f)(std::forward<Args>(args)...)) ret_value;
    std::string message;
    bool has_thrown = false;
    try
    {
      ret_value = (obj.*f)(std::forward<Args>(args)...);
    }
    catch(Json::RuntimeError& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(Json::LogicError& error)
    {
      has_thrown = true;

      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(ParseError& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(std::ios_base::failure& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(CompressionError& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(std::runtime_error& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(std::invalid_argument& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(...)
    {
      has_thrown = true;
      message = "An unexpected error occured!";
      if(throw_exceptions_)
        throw std::runtime_error("An unexpected error occured!");
    }

    if(has_thrown)
    {
      emit sendError("Error", message.c_str());
      return {};
    }
    return ret_value;
  }

  /*!
   * \brief Wrapper for generic functions. Catches specific exception types and sends an error
   * message to the gui if an exception was thrown.
   * \param f Function to run.
   * \param args Arguments for the function.
   * \return If no exception was thrown: The return value of the function,
   * else: An empty optional.
   */
  template<typename Func, typename... Args>
  auto handleExceptionsForFunction(Func&& f, Args&&... args)
    -> std::optional<decltype((f)(std::forward<Args>(args)...))>
  {
    decltype((f)(std::forward<Args>(args)...)) ret_value;
    std::string message;
    bool has_thrown = false;
    try
    {
      ret_value = (f)(std::forward<Args>(args)...);
    }
    catch(Json::RuntimeError& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(Json::LogicError& error)
    {
      has_thrown = true;

      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(ParseError& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(std::ios_base::failure& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(CompressionError& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(std::runtime_error& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(std::invalid_argument& error)
    {
      has_thrown = true;
      message = error.what();
      if(throw_exceptions_)
        throw error;
    }
    catch(...)
    {
      has_thrown = true;
      message = "An unexpected error occured!";
      if(throw_exceptions_)
        throw std::runtime_error("An unexpected error occured!");
    }

    if(has_thrown)
    {
      emit sendError("Error", message.c_str());
      return {};
    }
    return ret_value;
  }

  /*! \brief Contains every ModdedApplication handled by this object. */
  std::vector<ModdedApplication> apps_;
  /*! \brief If true: Do not catch exceptions. */
  bool throw_exceptions_ = false;

  /*!
   * \brief Updates the settings file with the current state of this object.
   */
  void updateSettings();
  /*!
   * \brief Updates the internal state of this object to the state stored in the settings file.
   */
  void updateState();
  /*!
   * \brief Checks if given app_id is part of apps_ and optionally emits an error signal.
   * \param app_id Target app id.
   * \param show_error If true: Emit \ref sendError.
   * if app id is invalid.
   * \return True if app id is valid, else false.
   */
  bool appIndexIsValid(int app_id, bool show_error = true);
  /*!
   * \brief Checks if given deployer id is valid for given app and optionally emits
   * an error signal.
   * \param app_id Target app id.
   * \param deployer Target deployer.
   * \param show_error If true: Emit \ref sendError.
   * if deployer id is invalid.
   * \return True if deployer id is valid, else false.
   */
  bool deployerIndexIsValid(int app_id, int deployer, bool show_error = true);
  /*!
   * \brief If the code indicates an error: Create an error message and emit it
   * using \ref sendError.
   * \param code The error code.
   * \param staging_dir The \ref ModdedApplication "application"s staging directory.
   */
  void handleAddAppError(int code, std::filesystem::path staging_dir);
  /*!
   * \brief If the code indicates an error: Create an error message and emit it
   * using \ref sendError.
   * \param code The error code.
   * \param staging_dir The \ref ModdedApplication "application's" staging directory.
   * \param dest_dir The Deployer's target directory.
   * \param error_message A more detailed error message (if an error occured).
   */
  void handleAddDeployerError(int code,
                              std::filesystem::path staging_dir,
                              std::filesystem::path dest_dir,
                              const std::string& error_message);
  /*!
   * \brief Emits an error message indicating a parsing error using \ref sendError.
   * \param path Path to the file causing this error.
   * \param message Message of the exception thrown during parsing.
   */
  void handleParseError(std::string path, std::string message);

  /*! \brief Counter for the number of instances of this class. */
  inline static int number_of_instances_ = 0;

private:
  /*!
   * \brief Emits logMessage with the given data.
   * \param level Log level.
   * \param message Log message.
   */
  void sendLogMessage(Log::LogLevel level, QString message);

signals:
  /*!
   * \brief Sends the names of all deployers.
   * \param names The deployer names.
   * \param is_new Indicates whether this signal was emitted after adding a new deployer.
   */
  void sendDeployerNames(QStringList names, bool is_new);
  /*!
   *  \brief Sends ModInfo for one \ref ModdedApplication "application".
   *  \param mod_info The mod info.
   */
  void sendModInfo(std::vector<ModInfo> mod_info);
  /*!
   * \brief Sends the load order for one deployer of one \ref ModdedApplication "application".
   * \param loadorder The load order.
   */
  void sendLoadorder(std::vector<std::tuple<int, bool>> loadorder);
  /*!
   * \brief Sends DeployerInfo for one deployer of one \ref ModdedApplication "application".
   * \param depl_info The DeployerInfo.
   */
  void sendDeployerInfo(DeployerInfo depl_info);
  /*!
   *  \brief Sends a list containing all \ref ModdedApplication "application" names.
   *  \param names The list of names.
   *  \param icon_paths Paths to application icons.
   *  \param is_new Indicates whether this was emitted after adding a new \ref
   * ModdedApplication "application".
   */
  void sendApplicationNames(QStringList names, QStringList icon_paths, bool is_new);
  /*!
   *  \brief Emitted after potentially slow operations, e.g. installing a mod, are completed.
   *  \param message Status message to show in the main window.
   */
  void completedOperations(QString message = "");
  /*!
   * \brief Sends file conflicts for one mod for one deployer of one \ref ModdedApplication
   * "application".
   * \param conflicts A vector containing the conflicts information.
   */
  void sendFileConflicts(std::vector<ConflictInfo> conflicts);
  /*!
   * \brief Sends AppInfo for one \ref ModdedApplication "application".
   * \param The AppInfo.
   */
  void sendAppInfo(AppInfo app_info);
  /*!
   * \brief Sends mod conflicts for one mod for one deployer of one \ref ModdedApplication
   * "application".
   * \param Contains every mod id in conflict.
   */
  void sendModConflicts(std::unordered_set<int> conflicts);
  /*!
   * \brief Sends a list of all profile names for one \ref ModdedApplication "application".
   * \param names The profile names.
   * \param is_new Indicates whether this was emitted after adding a new profile.
   */
  void sendProfileNames(QStringList names, bool is_new);
  /*!
   * \brief Sends an error message.
   * \param title The title of the message window.
   * \param message The error message.
   */
  void sendError(QString title, QString message);
  /*!
   * \brief Emitted after archive extraction is complete.
   * \param info Contains extraction directory as ImportModInfo::current_path.
   */
  void extractionComplete(ImportModInfo info);
  /*!
   * \brief Sends a log message to the logging window.
   * \param log_level Type of message.
   * \param message Message to be displayed.
   */
  void logMessage(Log::LogLevel log_level, QString message);
  /*!
   * \brief Sends a vector containing info about all backup targets managed by given
   * ModdedApplication.
   * \param targets The targets.
   */
  void sendBackupTargets(std::vector<BackupTarget> targets);
  /*! \brief Used to synchronize scrolling in lists with the event queue. */
  void scrollLists();
  /*!
   * \brief Informs about the progress in the current task.
   * \param progress The progress.
   */
  void updateProgress(float progress);
  /*!
   * \brief Sends NexusMods data for a specific mod.
   * \param app_id App to which the mod belongs.
   * \param mod_id Target mod id.
   * \param page Contains all data for the mod.
   */
  void sendNexusPage(int app_id, int mod_id, nexus::Page page);
  /*!
   * \brief Signals successful completion of a mod download.
   * \param info ImportModInfo::local_source contains the download directory.
   */
  void downloadComplete(ImportModInfo info);
  /*! \brief Signals a failed download. */
  void downloadFailed();
  /*!
   * \brief Signals mod installation has been completed.
   * \param success If true: Installation was successful.
   */
  void modInstallationComplete(bool success);
  /*!
   * \brief Sends data about externally modified files for one app for one deployer.
   * \param app_id Target app.
   * \param info Contains data about modified files.
   * \param num_deployers The total number of deployers for the target app.
   * \param deploy If True: Deploy mods after checking, else: Undeploy mods.
   */
  void sendExternalChangesInfo(int app_id, ExternalChangesInfo info, int num_deployers, bool deploy);
  /*!
   * \brief Signals that external changes to files for given app for given deployer have been
   * handled.
   * \param app_id Target app.
   * \param deployer Target deployer.
   * \param num_deployers The total number of deployers for the target app.
   * \param deploy If True: Deploy mods after checking, else: Undeploy mods.
   */
  void externalChangesHandled(int app_id, int deployer, int num_deployers, bool deploy);

public slots:
  /*!
   * \brief Adds a new \ref ModdedApplication "application".
   * \param info Contains all data needed to add a new application, e.g. its name.
   */
  void addApplication(EditApplicationInfo info);
  /*!
   * \brief Removes an \ref ModdedApplication "application" and optionally deletes all
   * installed mods and the settings file in the \ref ModdedApplication "application's" staging
   * directory.
   * \param app_id The target \ref ModdedApplication "application".
   * \param cleanup Indicates if mods and settings file should be deleted.
   */
  void removeApplication(int app_id, bool cleanup);
  /*!
   * \brief Deploys mods using all Deployer objects of one \ref ModdedApplication
   * "application".
   * \param app_id The target \ref ModdedApplication "application".
   */
  void deployMods(int app_id);
  /*!
   * \brief Deploys mods for given deployers and given application.
   * \param app_id Target application.
   * \param deployer_ids Target deployers.
   */
  void deployModsFor(int app_id, std::vector<int> deployer_ids);
  /*!
   * \brief Undeploys mods using all Deployer objects of one \ref ModdedApplication
   * "application".
   * \param app_id The target \ref ModdedApplication "application".
   */
  void unDeployMods(int app_id);
  /*!
   * \brief Undeploys mods for given deployers and given application.
   * \param app_id Target application.
   * \param deployer_ids Target deployers.
   */
  void unDeployModsFor(int app_id, std::vector<int> deployer_ids);
  /*!
   * \brief Installs a new mod for one \ref ModdedApplication "application" using
   * the given Installer type.
   * \param app_id The target \ref ModdedApplication "application".
   * \param info Contains all data needed to install the mod.
   */
  void installMod(int app_id, ImportModInfo info);
  /*!
   * \brief Uninstalls the given mods for one \ref ModdedApplication "application", this includes
   * deleting all installed files.
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_ids Ids of the mods to be uninstalled.
   * \param installer_type The
   * Installer type used. If an empty string is given, the Installer used during installation
   * is used.
   */
  void uninstallMods(int app_id, std::vector<int> mod_ids, std::string installer_type);
  void commitChanges(int app_id, int deployer);
  /*!
   * \brief Updates which \ref Deployer "deployer" should manage given mods.
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_id Vector of mod ids to be added.
   * \param deployers Bool for every deployer, indicating if the mods should be managed
   * by that deployer.
   */
  void updateModDeployers(int app_id, std::vector<int> mod_ids, std::vector<bool>);
  /*!
   * \brief Removes a mod from the load order for given Deployer for
   * given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer The target Deployer
   * \param mod_id Id of the mod to be removed.
   */
  void removeNodeFromDeployer(int app_id, int deployer, void *node_ptr);
  /*!
   * \brief Enables or disables the given mod in the load order for given Deployer
   * for given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer The target Deployer
   * \param mod_id Mod to be edited.
   * \param status The new status.
   */
  void setModStatus(int app_id, int deployer, int mod_id, bool status);
  /*!
   * \brief Adds a new Deployer of given type to given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param info Contains all data needed to add a new deployer, e.g. its name.
   */
  void addDeployer(int app_id, EditDeployerInfo info);
  /*!
   * \brief Removes a Deployer from an \ref ModdedApplication "application".
   * \param app_id Target \ref ModdedApplication "application".
   * \param deployer Target Deployer.
   * \param cleanup If true: Remove all currently deployed files and restore backups.
   */
  void removeDeployer(int app_id, int deployer, bool cleanup);
  /*!
   * \brief Creates a vector containing the names of all Deployer objects for one \ref
   * ModdedApplication "application". Emits \ref sendDeployerNames.
   * \param app_id The target \ref ModdedApplication "application".
   * \param is_new Indicates if this was called after a new deployer was added.
   */
  void getDeployerNames(int app_id, bool is_new);
  /*!
   * \brief Creates a vector containing information about all installed mods, stored in ModInfo
   * objects for one \ref ModdedApplication "application". Emits \ref sendModInfo.
   * \param app_id The target \ref ModdedApplication "application".
   */
  void getModInfo(int app_id);
  /*!
   * \brief Creates DeployerInfo for one Deployer for one \ref ModdedApplication "application".
   * Emits \ref sendDeployerInfo.
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer Target deployer.
   */
  void getDeployerInfo(int app_id, int deployer);
  /*!
   * \brief Emits sendApplicationNames.
   * \param is_new Indicates whether this was called after adding a new \ref ModdedApplication
   * "application".
   */
  void getApplicationNames(bool is_new);
  /*!
   * \brief Setter for a mod name.
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_id Target mod.
   * \param new_name The new name.
   */
  void changeModName(int app_id, int mod_id, QString new_name);
  /*!
   * \brief Checks for file conflicts of given mod with all other mods in the load order for
   * one Deployer of one \ref ModdedApplication "application". Emits \ref sendFileConflicts
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer The target Deployer
   * \param mod_id Mod to be checked.
   * \param show_disabled If true: Also check for conflicts with disabled mods.
   */
  void getFileConflicts(int app_id, int deployer, int mod_id, bool show_disabled);
  /*!
   * \brief Creates AppInfo for given \ref ModdedApplication "application".
   * Emits \ref sendAppInfo.
   * \param app_id The target \ref ModdedApplication "application".
   */
  void getAppInfo(int app_id);
  /*!
   * \brief Adds a new tool to given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param tool The new Tool.
   */
  void addTool(int app_id, Tool tool);
  /*!
   * \brief Removes a tool from given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param tool_id The tool's id.
   */
  void removeTool(int app_id, int tool_id);
  /*!
   * \brief Edits an \ref ModdedApplication "application" and optionally moves all
   *  of it's mods to a new directory.
   * \param info Contains all data needed to edit an application, e.g. its new name.
   * \param app_id The target \ref ModdedApplication "application".
   */
  void editApplication(EditApplicationInfo info, int app_id);
  /*!
   * \brief Used to set type, name and target directory for one deployer of one
   * \ref ModdedApplication "application".
   * \param info Contains all data needed to edit a deployer, e.g. its new name.
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer Target Deployer.
   */
  void editDeployer(EditDeployerInfo info, int app_id, int deployer);
  /*!
   * \brief Checks for conflicts with other mods for one Deployer of
   * one \ref ModdedApplication "application".
   * Two mods are conflicting if they share at least one file. Emits \ref sendModConflicts.
   * \param app_id The target \ref ModdedApplication "application".
   * \param deployer Target Deployer.
   * \param mod_id The mod to be checked.
   */
  void getModConflicts(int app_id, int deployer, int mod_id);
  /*!
   * \brief Sets the currently active profile for given \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param profile The new profile.
   */
  void setProfile(int app_id, int profile);
  /*!
   * \brief Adds a new profile to one \ref ModdedApplication "application" and optionally
   * copies it's load order from an existing profile.
   * \param app_id The target \ref ModdedApplication "application".
   * \param info Contains data for the new profile.
   */
  void addProfile(int app_id, EditProfileInfo info);
  /*!
   * \brief Removes a profile from an \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param profile The profile to be removed.
   */
  void removeProfile(int app_id, int profile);
  /*!
   * \brief Creates a vector containing the names of all profiles of one
   * \ref ModdedApplication "application". Emits \ref sendProfileNames.
   * \param app_id The target \ref ModdedApplication "application".
   */
  void getProfileNames(int app_id, bool is_new);
  /*!
   * \brief Used to set the name of a profile for one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param profile Target Profile
   * \param info Contains the new data for the profile.
   */
  void editProfile(int app_id, int profile, EditProfileInfo info);
  /*!
   * \brief Used to replace an existing to with a now one for a \ref ModdedApplication
   * "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param tool_id Target tool.
   * \param new_tool The new tool.
   */
  void editTool(int app_id, int tool_id, Tool new_tool);
  /*!
   * \brief Adds a mod to an existing group of an \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_id The mod's id.
   * \param group The target group.
   */
  void addModToGroup(int app_id, int mod_id, int group);
  /*!
   * \brief Removes a mod from it's group for one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_id Target mod.
   */
  void removeModFromGroup(int app_id, int mod_id);
  /*!
   * \brief Creates a new group containing the two given mods for one
   * \ref ModdedApplication "application". A group is a set of mods where only one member,
   * the active member, will be deployed.
   * \param app_id The target \ref ModdedApplication "application".
   * \param first_mod_id First mod. This will be the active member of the new group.
   * \param second_mod_id Second mod.
   */
  void createGroup(int app_id, int first_mod_id, int second_mod_id);
  /*!
   * \brief Changes the active member of given group of an
   * \ref ModdedApplication "application" to given mod.
   * \param app_id The target \ref ModdedApplication "application".
   * \param group Target group.
   * \param mod_id The new active member.
   */
  void changeActiveGroupMember(int app_id, int group, int mod_id);
  /*!
   * \brief Sets the given mod's version to the given new version for
   * one \ref ModdedApplication "application".
   * \param app_id The target \ref ModdedApplication "application".
   * \param mod_id Target mod.
   * \param new_version The new version.
   */
  void changeModVersion(int app_id, int mod_id, QString new_version);
  /*!
   * \brief Sorts the load order by grouping mods which contain conflicting files.
   * \param app_id Target app.
   * \param deployer Target deployer.
   */
  void sortModsByConflicts(int app_id, int deployer);
  /*!
   * \brief Extracts the given archive to the given location.
   * \param info Contains archive source path in ImportModInfo::local_source
   * and extraction target path in ImportModInfo::target_path.
   */
  void extractArchive(ImportModInfo info);
  /*!
   * \brief Adds a new target file or directory to be managed by the BackupManager of given
   * ModdedApplication.
   * \param app_id Target app.
   * \param path Path to the target file or directory.
   * \param name Display name for this target.
   * \param default_backup Display name for the currently active version of the target.
   * \param first_backup If not empty: Create a backup of the target with this as name.
   */
  void addBackupTarget(int app_id,
                       QString path,
                       QString name,
                       QString default_backup,
                       QString first_backup);
  /*!
   * \brief Removes the given backup target from the given ModdedApplication by deleting
   * all relevant backups and config files.
   * \param app_id Target app.
   * \param target_id Target to remove.
   */
  void removeBackupTarget(int app_id, int target_id);
  /*!
   * \brief Adds a new backup for the given target for the given ModdedApplication by copying
   * the currently active backup.
   * \param app_id Target app.
   * \param target_id Target for which to create a new backup.
   * \param name Display name for the new backup.
   * \param source Backup from which to copy files to create the new backup. If -1:
   * copy currently active backup.
   */
  void addBackup(int app_id, int target_id, QString name, int source);
  /*!
   * \brief Deletes the given backup for given target for given ModdedApplication.
   * \param app_id Target app.
   * \param target_id Target from which to delete a backup.
   * \param backup_id Backup to remove.
   */
  void removeBackup(int app_id, int target_id, int backup_id);
  /*!
   * \brief Changes the currently active backup for the given target for the given
   * ModdedApplication.
   * \param app_id Target app.
   * \param target_id Target for which to change the active backup.
   * \param backup_id New active backup.
   */
  void setActiveBackup(int app_id, int target_id, int backup_id);
  /*!
   * \brief Returns a vector containing information about all managed backup targets of
   * given ModdedApplication. Emits \ref sendBackupTargets
   * \param app_id Target app.
   */
  void getBackupTargets(int app_id);
  /*!
   * \brief Changes the name of the given backup for the given target for the given
   * ModdedApplication.
   * \param app_id Target app.
   * \param target_id Backup target.
   * \param backup_id Backup to be edited.
   * \param name The new name.
   */
  void setBackupName(int app_id, int target_id, int backup_id, QString name);
  /*!
   * \brief Changes the name of the given backup target for the given
   * ModdedApplication.
   * \param app_id Target app.
   * \param target_id Backup target.
   * \param name The new name.
   */
  void setBackupTargetName(int app_id, int target_id, QString name);
  /*!
   * \brief Deletes all files in the dest backup and replaces them with the files
   * from the source backup for the given ModdedApplication.
   * \param app_id Target app.
   * \param target_id Backup target.
   * \param source_backup Backup from which to copy files.
   * \param dest_backup Target for data deletion.
   */
  void overwriteBackup(int app_id, int target_id, int source_backup, int dest_backup);
  /*! \brief Used to synchronize scrolling in lists with the event queue. */
  void onScrollLists();
  /*!
   * \brief Uninstalls all mods which are inactive group members of any group which contains
   * any of the given mods for the given ModdedApplication.
   * \param app_id Target app.
   * \param mod_ids Ids of the mods for which to uninstall group members.
   */
  void uninstallGroupMembers(int app_id, const std::vector<int>& mod_ids);
  /*!
   * \brief Adds a new tag with the given name to the given ModdedApplication.
   * Fails if a tag by that name already exists.
   * \param app_id Target app.
   * \param tag_name Name for the new tag.
   * \throw std::runtime_error If a tag by that name exists.
   */
  void addManualTag(int app_id, QString tag_name);
  /*!
   * \brief Removes the tag with the given name, if it exists, from the given ModdedApplication.
   * \param app_id Target app.
   * \param tag_name Tag to be removed.
   */
  void removeManualTag(int app_id, QString tag_name);
  /*!
   * \brief Changes the name of the given tag to the given new name for the given ModdedApplication.
   * Fails if a tag by the given name exists.
   * \param app_id Target app.
   * \param old_name Name of the target tag.
   * \param new_name Target tags new name.
   * \throw std::runtime_error If a tag with the given new_name exists.
   */
  void changeManualTagName(int app_id, QString old_name, QString new_name);
  /*!
   * \brief Adds the given tag to all given mods for the given ModdedApplication.
   * \param app_id Target app.
   * \param tag_name Target tags name.
   * \param mod_ids Target mod ids.
   */
  void addTagsToMods(int app_id, QStringList tag_names, const std::vector<int>& mod_ids);
  /*!
   * \brief Removes the given tag from the given mods for the given ModdedApplication.
   * \param app_id Target app.
   * \param tag_name Target tags name.
   * \param mod_ids Target mod ids.
   */
  void removeTagsFromMods(int app_id, QStringList tag_names, const std::vector<int>& mod_ids);
  /*!
   * \brief Sets the tags for all given mods to the given tags for the given ModdedApplication.
   * \param app_id Target app.
   * \param tag_names Names of the new tags.
   * \param mod_ids Target mod ids.
   */
  void setTagsForMods(int app_id, QStringList tag_names, const std::vector<int>& mod_ids);
  /*!
   * \brief Performes the given tag editing actions for the given ModdedApplication.
   * \param app_id Target app.
   * \param actions Editing actions.
   */
  void editManualTags(int app_id, std::vector<EditManualTagAction> actions);
  /*!
   * \brief Performes the given tag editing actions for the given ModdedApplication.
   * \param app_id Target app.
   * \param actions Editing actions.
   */
  void editAutoTags(int app_id, std::vector<EditAutoTagAction> actions);
  /*!
   * \brief Reapplies all auto tags for all mods for the given ModdedApplication.
   * \param app_id Target app.
   */
  void reapplyAutoTags(int app_id);
  /*!
   * \brief Reapplies all auto tags to the given mods for the given ModdedApplication.
   * \param app_id Target app.
   * \param mod_ids Ids of the mods to which auto tags are to be reapplied.
   */
  void updateAutoTags(int app_id, std::vector<int> mod_ids);
  /*!
   * \brief Sets a mods local and remote source to the given values for the given ModdedApplication.
   * \param app_id App to which the edited mod belongs.
   * \param mod_id Target mod id.
   * \param local_source Path to a local archive or directory used for mod installation.
   * \param remote_source Remote URL from which the mod was downloaded.
   */
  void editModSources(int app_id, int mod_id, QString local_source, QString remote_source);
  /*!
   * \brief Fetches data for the given mod from NexusMods.
   * \param app_id App to which the mod belongs.
   * \param mod_id Target mod id.
   */
  void getNexusPage(int app_id, int mod_id);
  /*!
   * \brief Downloads a mod from nexusmods
   *
   * The mod is downloaded to the target apps staging directory. Uses either the given nxm URL
   * or the mod and file id for the download.
   *
   * \param info Contains either a nxm URL or nexus mod and file ids.
   */
  void downloadMod(ImportModInfo info);
  /*!
   * \brief Checks for available mod updates on NexusMods.
   * \param app_id App for which mod updates are to be checked.
   */
  void checkForModUpdates(int app_id);
  /*!
   * \brief Checks for available updates for the given mod for the given app.
   * \param app_id Target app.
   * \param mod_ids Ids of the mods for which to check for updates.
   */
  void checkModsForUpdates(int app_id, const std::vector<int>& mod_ids);
  /*!
   * \brief Temporarily disables update notifications for the given mods.
   * \param app_id Target app.
   * \param mod_ids Ids of the mods for which update notifications are to be disabled.
   */
  void suppressUpdateNotification(int app_id, const std::vector<int>& mod_ids);
  /*!
   * \brief Checks if files deployed by the given app by the given deployer have
   * been externally overwritten.
   * \param app_id Target app.
   * \param deployer Deployer to check.
   * \param deploy If True: Deploy mods after checking, else: Undeploy mods.
   */
  void getExternalChanges(int app_id, int deployer, bool deploy);
  /*!
   * \brief Keeps or reverts external changes for one app for one deployer.
   * For every given file: Moves the modified file into the source mods directory and links
   * it back in, if the changes are to be kept. Else: Deletes that file and restores
   * the original link.
   * \param app_id Target app.
   * \param deployer Target deployer.
   * \param changes_to_keep Contains paths to modified files, the id of the mod currently
   * responsible for that file and a bool which indicates whether or not changes to
   * that file should be kept.
   * \param deploy If True: Deploy mods after checking, else: Undeploy mods.
   */
  void keepOrRevertFileModifications(
    int app_id,
    int deployer,
    const FileChangeChoices& changes_to_keep,
    bool deploy);
  /*!
   * \brief Exports configurations for the given deployers and the given auto tags to a json file.
   * Does not include mods.
   * \param app_id Target app.
   * \param deployers Deployers to export.
   * \param auto_tags Auto tags to export.
   */
  void exportAppConfiguration(int app_id, std::vector<int> deployers, QStringList auto_tags);
  /*!
   * \brief Updates the file ignore list for ReverseDeployers
   * \param app_id Target app.
   * \param deployer Target deployer.
   */
  void updateIgnoredFiles(int app_id, int deployer);
  /*!
   * \brief Adds the given mod to the ignore list of the given ReverseDeployer.
   * \param app_id Target app.
   * \param deployer Target deployer.
   * \param mod_id Mod to be ignored.
   */
  void addModToIgnoreList(int app_id, int deployer, int mod_id);
  /*!
   * \brief Applies the given mod action to the given mod.
   * \param app_id Target app.
   * \param deployer Target deployer.
   * \param action Action to be applied.
   * \param mod_id Target mod.
   */
  void applyModAction(int app_id, int deployer, int action, int mod_id);
};
