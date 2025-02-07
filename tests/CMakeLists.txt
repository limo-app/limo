# catch2
find_package(Catch2 3 REQUIRED)

set(TEST_SOURCES
        ../src/core/addmodinfo.h
        ../src/core/appinfo.h
        ../src/core/autotag.cpp
        ../src/core/autotag.h
        ../src/core/backupmanager.cpp
        ../src/core/backupmanager.h
        ../src/core/backuptarget.cpp
        ../src/core/backuptarget.h
        ../src/core/bg3deployer.cpp
        ../src/core/bg3deployer.h
        ../src/core/bg3pakfile.cpp
        ../src/core/bg3pakfile.h
        ../src/core/bg3plugin.cpp
        ../src/core/bg3plugin.h
        ../src/core/casematchingdeployer.cpp
        ../src/core/casematchingdeployer.h
        ../src/core/conflictinfo.h
        ../src/core/cryptography.cpp
        ../src/core/cryptography.h
        ../src/core/deployer.cpp
        ../src/core/deployer.h
        ../src/core/deployerfactory.cpp
        ../src/core/deployerfactory.h
        ../src/core/deployerinfo.h
        ../src/core/editapplicationinfo.h
        ../src/core/editautotagaction.cpp
        ../src/core/editautotagaction.h
        ../src/core/editdeployerinfo.h
        ../src/core/editmanualtagaction.cpp
        ../src/core/editmanualtagaction.h
        ../src/core/editprofileinfo.h
        ../src/core/filechangechoices.h
        ../src/core/fomod/dependency.cpp
        ../src/core/fomod/dependency.h
        ../src/core/fomod/file.h
        ../src/core/fomod/fomodinstaller.cpp
        ../src/core/fomod/fomodinstaller.h
        ../src/core/fomod/plugin.h
        ../src/core/fomod/plugindependency.h
        ../src/core/fomod/plugingroup.h
        ../src/core/fomod/plugintype.h
        ../src/core/importmodinfo.h
        ../src/core/installer.cpp
        ../src/core/installer.h
        ../src/core/log.cpp
        ../src/core/log.h
        ../src/core/lootdeployer.cpp
        ../src/core/lootdeployer.h
        ../src/core/lspakextractor.cpp
        ../src/core/lspakextractor.h
        ../src/core/lspakfilelistentry.h
        ../src/core/lspakheader.h
        ../src/core/manualtag.cpp
        ../src/core/manualtag.h
        ../src/core/mod.cpp
        ../src/core/mod.h
        ../src/core/moddedapplication.cpp
        ../src/core/moddedapplication.h
        ../src/core/modinfo.h
        ../src/core/nexus/api.cpp
        ../src/core/nexus/api.h
        ../src/core/nexus/file.cpp
        ../src/core/nexus/file.h
        ../src/core/nexus/mod.cpp
        ../src/core/nexus/mod.h
        ../src/core/openmwarchivedeployer.cpp
        ../src/core/openmwarchivedeployer.h
        ../src/core/openmwplugindeployer.cpp
        ../src/core/openmwplugindeployer.h
        ../src/core/parseerror.h
        ../src/core/pathutils.cpp
        ../src/core/pathutils.h
        ../src/core/plugindeployer.cpp
        ../src/core/plugindeployer.h
        ../src/core/progressnode.cpp
        ../src/core/progressnode.h
        ../src/core/reversedeployer.cpp
        ../src/core/reversedeployer.h
        ../src/core/tag.cpp
        ../src/core/tag.h
        ../src/core/tagcondition.h
        ../src/core/tagconditionnode.cpp
        ../src/core/tagconditionnode.h
        ../src/core/tool.cpp
        ../src/core/tool.h
        test_backupmanager.cpp
        test_bg3deployer.cpp
        test_cryptography.cpp
        test_deployer.cpp
        test_fomodinstaller.cpp
        test_installer.cpp
        test_lootdeployer.cpp
        test_moddedapplication.cpp
        test_openmwdeployer.cpp
        test_reversedeployer.cpp
        test_tagconditionnode.cpp
        test_tool.cpp
        test_utils.cpp
        test_utils.h
        tests.cpp
)
     
configure_file(test_utils.h.in test_utils.h)
     
add_executable(tests ${TEST_SOURCES})
target_include_directories(tests
        PUBLIC "${PROJECT_BINARY_DIR}"
        PRIVATE ${LibArchive_INCLUDE_DIRS}
        PRIVATE ${LIBLOOT_INCLUDE_DIR}
        PRIVATE ${JSONCPP_INCLUDE_DIRS}
        PRIVATE ${LIBUNRAR_INCLUDE_DIR}
        PRIVATE ${LZ4_INCLUDE_DIRS}
        PRIVATE ${ZSTD_INCLUDE_DIRS})
target_link_libraries(tests
PRIVATE Catch2::Catch2WithMain
    PRIVATE ${JSONCPP_LIBRARIES}
    PRIVATE ${LibArchive_LIBRARIES}
    PRIVATE ${LIBLOOT_PATH}
    PRIVATE cpr::cpr
    PRIVATE OpenSSL::SSL
    PRIVATE ${LIBUNRAR_PATH}
    PRIVATE ${LZ4_LIBRARIES}
    PRIVATE ${ZSTD_LIBRARIES}
    PRIVATE ZLIB::ZLIB
    PRIVATE pugixml::pugixml)

include(Catch)
catch_discover_tests(tests)
