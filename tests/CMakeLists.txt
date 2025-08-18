# catch2
find_package(Catch2 3 REQUIRED)

set(TEST_SOURCES
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
        matcher.cpp
        matcher.h
        tests.cpp
)

configure_file(test_utils.h.in test_utils.h)

add_executable(tests ${TEST_SOURCES})
target_include_directories(tests
    PRIVATE core
)
target_link_libraries(tests
    PRIVATE Catch2::Catch2WithMain
    PRIVATE core
)

include(Catch)
catch_discover_tests(tests)
