cmake_minimum_required(VERSION 3.25)
project(LMM_Tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

option(USE_SYSTEM_LIBUNRAR "Whether to use the system version of libunrar." ON)
   
# libarchive
find_package(LibArchive REQUIRED)

# jsoncpp
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

# pugixml
find_package(pugixml REQUIRED)

# catch2
find_package(Catch2 3 REQUIRED)

# cpr
find_package(cpr REQUIRED)

# OpenSSL
find_package(OpenSSL REQUIRED)

# libunrar
if(NOT DEFINED LIBUNRAR_INCLUDE_DIR)
    if(IS_FLATPAK)
        set(LIBUNRAR_INCLUDE_DIR "/app/include/unrar")
    else()
        if(USE_SYSTEM_LIBUNRAR)
            find_file(LIBUNRAR_DLL_PATH "unrar/dll.hpp" REQUIRED)
            cmake_path(GET LIBUNRAR_DLL_PATH PARENT_PATH LIBUNRAR_INCLUDE_DIR)
        else()
            set(LIBUNRAR_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/unrar")
        endif()
    endif()
endif()
if(NOT DEFINED LIBUNRAR_PATH)
    if(IS_FLATPAK)
        set(LIBUNRAR_PATH "/app/lib/libunrar.a")
    else()
        if(USE_SYSTEM_LIBUNRAR)
            find_library(LIBUNRAR_PATH libunrar.a)
            if(${LIBUNRAR_PATH} STREQUAL "LIBUNRAR_PATH-NOTFOUND")
                find_library(LIBUNRAR_PATH libunrar.so REQUIRED)
            endif()
        else()
            set(LIBUNRAR_PATH "${PROJECT_SOURCE_DIR}/unrar/libunrar.a")
        endif()
    endif()
endif()

# libloot
if(NOT DEFINED LIBLOOT_INCLUDE_DIR)
    find_file(LIBLOOT_API_PATH "loot/api.h" REQUIRED)
    cmake_path(GET LIBLOOT_API_PATH PARENT_PATH LIBLOOT_INCLUDE_DIR)
endif()
if(NOT DEFINED LIBLOOT_PATH)
    find_library(LIBLOOT_PATH libloot.so REQUIRED)
endif()

# lz4
if(NOT DEFINED LZ4_INCLUDE_DIR)
    find_file(LZ4_HEADER_PATH "lz4.h" REQUIRED)
    cmake_path(GET LZ4_HEADER_PATH PARENT_PATH LZ4_INCLUDE_DIR)
endif()
if(NOT DEFINED LZ4_PATH)
    find_library(LZ4_PATH liblz4.so REQUIRED)
endif()

# zstd
if(NOT DEFINED ZSTD_INCLUDE_DIR)
    find_file(ZSTD_HEADER_PATH "zstd.h" REQUIRED)
    cmake_path(GET ZSTD_HEADER_PATH PARENT_PATH ZSTD_INCLUDE_DIR)
endif()
if(NOT DEFINED ZSTD_PATH)
    find_library(ZSTD_PATH libzstd.a REQUIRED)
endif()

# zlib
find_package(ZLIB REQUIRED)


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
        PRIVATE ${LZ4_INCLUDE_DIR}
        PRIVATE ${ZSTD_INCLUDE_DIR})
target_link_libraries(tests
PRIVATE Catch2::Catch2WithMain
    PRIVATE ${JSONCPP_LIBRARIES}
    PRIVATE ${LibArchive_LIBRARIES}
    PRIVATE ${LIBLOOT_PATH}
    PRIVATE cpr::cpr
    PRIVATE OpenSSL::SSL
    PRIVATE ${LIBUNRAR_PATH}
    PRIVATE ${LZ4_PATH}
    PRIVATE ${ZSTD_PATH}
    PRIVATE ZLIB::ZLIB
    PRIVATE pugixml::pugixml)
