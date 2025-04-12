/**
 * \file main.cpp
 * \brief Contains the main function
 */

#include "ui/ipcclient.h"
#include "ui/mainwindow.h"
#include <QApplication>
#include <filesystem>
#include <iostream>


/*!
 * \brief Main function of Limo.
 * \param argc Number of arguments passed to the application.
 * \param argv Array of arguments passed to the application.
 * \return 0: Application exited normally. 1: An error occurred while parsing arguments.
 * 2: Execution canceled, another Limo instance is already running.
 */
int main(int argc, char* argv[])
{
  QCoreApplication::setApplicationName("Limo");
  QApplication app(argc, argv);
  QIcon::setFallbackSearchPaths(
    QIcon::fallbackSearchPaths()
    << (std::filesystem::path(__FILE__).parent_path().parent_path() / "resources").c_str());
  QCommandLineParser parser;
  parser.setApplicationDescription("A simple tool for managing mods.");
  parser.addHelpOption();
  QCommandLineOption list_option(QStringList() << "l" << "list",
                                 "List all applications and their profiles.");
  QCommandLineOption deploy_option(QStringList() << "d" << "deploy",
                                   "Deploy all mods for given <application>. Requires setting "
                                   "a profile",
                                   "application");
  QCommandLineOption profile_option(
    QStringList() << "p" << "profile", "Set a <profile> to use for deployment.", "profile");
  QCommandLineOption debug_option(QStringList() << "D" << "debug" << "Show debug log messages.");
  parser.addOption(list_option);
  parser.addOption(deploy_option);
  parser.addOption(profile_option);
  parser.addOption(debug_option);
  parser.addPositionalArgument("url", "Imports the mod at this URL.");
  parser.process(app);
  const bool debug_mode = parser.isSet(debug_option);
  if(parser.isSet(list_option))
  {
    ApplicationManager app_man;
    app_man.enableExceptions(true);
    app_man.init();
    std::cout << app_man.toString();
    return 0;
  }
  if(parser.isSet(deploy_option))
  {
    bool is_int;
    QString input = parser.value(deploy_option);
    auto app_id = input.toInt(&is_int);
    if(!is_int)
    {
      std::cout << "Error: Specify the application id, '" << input.toStdString()
                << "' is not a number." << std::endl;
      return 1;
    }
    if(!parser.isSet(profile_option))
    {
      std::cout << "Error: Missing profile id." << std::endl;
      return 1;
    }
    input = parser.value(profile_option);
    int profile_id = input.toInt(&is_int);
    if(!is_int)
    {
      std::cout << "Error: Specify the profile id, '" << input.toStdString() << "' is not a number."
                << std::endl;
      return 1;
    }
    ApplicationManager app_man;
    app_man.enableExceptions(true);
    app_man.init();
    if(app_id < 0 || app_id >= app_man.getNumApplications())
    {
      std::cout << "Error: Application index out of bounds." << std::endl;
      return 1;
    }
    if(profile_id < 0 || profile_id >= app_man.getNumProfiles(app_id))
    {
      std::cout << "Error: Profile index out of bounds." << std::endl;
      return 1;
    }
    app_man.setProfile(app_id, profile_id);
    app_man.deployMods(app_id);
    return 0;
  }

  const auto pos_args = parser.positionalArguments();
  std::string argument = "";
  if(!pos_args.empty())
    argument = pos_args[0].toStdString();
  if(argument.starts_with('\"'))
    argument.erase(0, 1);
  if(argument.ends_with('\"'))
    argument.erase(argument.size() - 1, 1);
  IpcClient client;
  if(client.connect())
  {
    if(pos_args.empty())
    {
      client.sendString("Started");
      std::cout << "Another instance is already running. Sending arguments..." << std::endl;
      return 2;
    }
    std::regex nxm_regex(R"(nxm:\/\/.*\mods\/\d+\/files\/\d+\?.*)");
    std::smatch match;
    if(std::regex_match(argument, match, nxm_regex))
      client.sendString(argument);
    return 0;
  }







  // TODO: Remove
  std::cout << "Info size: " << sizeof(ImportModInfo) << std::endl;














  app.setWindowIcon(QIcon(":/logo.png"));
  MainWindow w;
  w.setDebugMode(debug_mode);
  if(!pos_args.empty())
    w.setCmdArgument(argument);
  emit w.getApplicationNames(false);
  w.show();
  w.initChangelog();
  return app.exec();
}
