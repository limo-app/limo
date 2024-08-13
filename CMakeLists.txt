cmake_minimum_required(VERSION 3.25)

project(Limo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(IS_FLATPAK "Whether this is being built for a flatpak." OFF)

# jsoncpp
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

# libarchive
find_package(LibArchive REQUIRED)

# pugixml
pkg_check_modules(PUGIXML pugixml)
find_package(pugixml REQUIRED)

# cpr
find_package(cpr REQUIRED)

# OpenSSL
find_package(OpenSSL REQUIRED)

# Qt
find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt5 REQUIRED COMPONENTS Widgets Svg Network)

set(PROJECT_SOURCES
        resources/icons.qrc
        src/core/addmodinfo.h
        src/core/appinfo.h
        src/core/autotag.cpp
        src/core/autotag.h
        src/core/backupmanager.cpp
        src/core/backupmanager.h
        src/core/backuptarget.cpp
        src/core/backuptarget.h
        src/core/casematchingdeployer.cpp
        src/core/casematchingdeployer.h
        src/core/compressionerror.h
        src/core/conflictinfo.h
        src/core/cryptography.cpp
        src/core/cryptography.h
        src/core/deployer.cpp
        src/core/deployer.h
        src/core/deployerfactory.cpp
        src/core/deployerfactory.h
        src/core/deployerinfo.h
        src/core/editapplicationinfo.h
        src/core/editautotagaction.cpp
        src/core/editautotagaction.h
        src/core/editdeployerinfo.h
        src/core/editmanualtagaction.cpp
        src/core/editmanualtagaction.h
        src/core/editprofileinfo.h
        src/core/fomod/dependency.cpp
        src/core/fomod/dependency.h
        src/core/fomod/file.h
        src/core/fomod/fomodinstaller.cpp
        src/core/fomod/fomodinstaller.h
        src/core/fomod/plugin.h
        src/core/fomod/plugindependency.h
        src/core/fomod/plugingroup.h
        src/core/fomod/plugintype.h
        src/core/importmodinfo.h
        src/core/installer.cpp
        src/core/installer.h
        src/core/log.cpp
        src/core/log.h
        src/core/lootdeployer.cpp
        src/core/lootdeployer.h
        src/core/manualtag.cpp
        src/core/manualtag.h
        src/core/mod.cpp
        src/core/mod.h
        src/core/moddedapplication.cpp
        src/core/moddedapplication.h
        src/core/modinfo.h
        src/core/nexus/api.cpp
        src/core/nexus/api.h
        src/core/nexus/file.cpp
        src/core/nexus/file.h
        src/core/nexus/mod.cpp
        src/core/nexus/mod.h
        src/core/parseerror.h
        src/core/pathutils.cpp
        src/core/pathutils.h
        src/core/progressnode.cpp
        src/core/progressnode.h
        src/core/tag.cpp
        src/core/tag.h
        src/core/tagcondition.h
        src/core/tagconditionnode.cpp
        src/core/tagconditionnode.h
        src/main.cpp
        src/ui/addapikeydialog.cpp
        src/ui/addapikeydialog.h
        src/ui/addapikeydialog.ui
        src/ui/addappdialog.cpp
        src/ui/addappdialog.h
        src/ui/addappdialog.ui
        src/ui/addautotagdialog.cpp
        src/ui/addautotagdialog.h
        src/ui/addautotagdialog.ui
        src/ui/addbackupdialog.cpp
        src/ui/addbackupdialog.h
        src/ui/addbackupdialog.ui
        src/ui/addbackuptargetdialog.cpp
        src/ui/addbackuptargetdialog.h
        src/ui/addbackuptargetdialog.ui
        src/ui/adddeployerdialog.cpp
        src/ui/adddeployerdialog.h
        src/ui/adddeployerdialog.ui
        src/ui/addmoddialog.cpp
        src/ui/addmoddialog.h
        src/ui/addmoddialog.ui
        src/ui/addprofiledialog.cpp
        src/ui/addprofiledialog.h
        src/ui/addprofiledialog.ui
        src/ui/addtodeployerdialog.cpp
        src/ui/addtodeployerdialog.h
        src/ui/addtodeployerdialog.ui
        src/ui/addtogroupdialog.cpp
        src/ui/addtogroupdialog.h
        src/ui/addtogroupdialog.ui
        src/ui/addtooldialog.cpp
        src/ui/addtooldialog.h
        src/ui/addtooldialog.ui
        src/ui/applicationmanager.cpp
        src/ui/applicationmanager.h
        src/ui/backuplistmodel.cpp
        src/ui/backuplistmodel.h
        src/ui/backuplistview.cpp
        src/ui/backuplistview.h
        src/ui/backupnamedelegate.cpp
        src/ui/backupnamedelegate.h
        src/ui/changeapipwdialog.cpp
        src/ui/changeapipwdialog.h
        src/ui/changeapipwdialog.ui
        src/ui/colors.h
        src/ui/conflictsmodel.cpp
        src/ui/conflictsmodel.h
        src/ui/deployerlistmodel.cpp
        src/ui/deployerlistmodel.h
        src/ui/deployerlistproxymodel.cpp
        src/ui/deployerlistproxymodel.h
        src/ui/deployerlistview.cpp
        src/ui/deployerlistview.h
        src/ui/editautotagsdialog.cpp
        src/ui/editautotagsdialog.h
        src/ui/editautotagsdialog.ui
        src/ui/editmanualtagsdialog.cpp
        src/ui/editmanualtagsdialog.h
        src/ui/editmanualtagsdialog.ui
        src/ui/editmodsourcesdialog.cpp
        src/ui/editmodsourcesdialog.h
        src/ui/editmodsourcesdialog.ui
        src/ui/enterapipwdialog.cpp
        src/ui/enterapipwdialog.h
        src/ui/enterapipwdialog.ui
        src/ui/fomodcheckbox.cpp
        src/ui/fomodcheckbox.h
        src/ui/fomoddialog.cpp
        src/ui/fomoddialog.h
        src/ui/fomoddialog.ui
        src/ui/fomodradiobutton.cpp
        src/ui/fomodradiobutton.h
        src/ui/importfromsteamdialog.cpp
        src/ui/importfromsteamdialog.h
        src/ui/importfromsteamdialog.ui
        src/ui/ipcclient.cpp
        src/ui/ipcclient.h
        src/ui/ipcserver.cpp
        src/ui/ipcserver.h
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
        src/ui/mainwindow.ui
        src/ui/managemodtagsdialog.cpp
        src/ui/managemodtagsdialog.h
        src/ui/managemodtagsdialog.ui
        src/ui/modlistmodel.cpp
        src/ui/modlistmodel.h
        src/ui/modlistproxymodel.cpp
        src/ui/modlistproxymodel.h
        src/ui/modlistview.cpp
        src/ui/modlistview.h
        src/ui/modnamedelegate.cpp
        src/ui/modnamedelegate.h
        src/ui/movemoddialog.cpp
        src/ui/movemoddialog.h
        src/ui/movemoddialog.ui
        src/ui/nexusmoddialog.cpp
        src/ui/nexusmoddialog.h
        src/ui/nexusmoddialog.ui
        src/ui/overwritebackupdialog.cpp
        src/ui/overwritebackupdialog.h
        src/ui/overwritebackupdialog.ui
        src/ui/passwordfield.cpp
        src/ui/passwordfield.h
        src/ui/settingsdialog.cpp
        src/ui/settingsdialog.h
        src/ui/settingsdialog.ui
        src/ui/tablecelldelegate.cpp
        src/ui/tablecelldelegate.h
        src/ui/tablepushbutton.cpp
        src/ui/tablepushbutton.h
        src/ui/tabletoolbutton.cpp
        src/ui/tabletoolbutton.h
        src/ui/tagcheckbox.cpp
        src/ui/tagcheckbox.h
        src/ui/validatinglineedit.cpp
        src/ui/validatinglineedit.h
        src/ui/versionboxdelegate.cpp
        src/ui/versionboxdelegate.h
)

add_executable(Limo
    ${PROJECT_SOURCES})
    
target_include_directories(Limo 
    PRIVATE "${PROJECT_SOURCE_DIR}/src"
    PRIVATE ${LibArchive_INCLUDE_DIRS}
    PRIVATE /usr/include/loot
    PRIVATE ${JSONCPP_INCLUDE_DIRS})

target_link_libraries(Limo
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE ${JSONCPP_LIBRARIES}
    PRIVATE ${LibArchive_LIBRARIES}
    PRIVATE ${PUGIXML_LIBRARIES}
    PRIVATE libloot.so
    PRIVATE Qt${QT_VERSION_MAJOR}::Svg
    PRIVATE cpr::cpr
    PRIVATE OpenSSL::SSL
    PRIVATE Qt${QT_VERSION_MAJOR}::Network)

install(TARGETS Limo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(IS_FLATPAK)
    install(FILES flatpak/io.github.limo_app.limo.png
            DESTINATION /app/share/icons/hicolor/512x512/apps)
    install(FILES flatpak/io.github.limo_app.limo.metainfo.xml
            DESTINATION /app/share/metainfo)
    install(FILES flatpak/io.github.limo_app.limo.desktop
            DESTINATION /app/share/applications)
endif()
