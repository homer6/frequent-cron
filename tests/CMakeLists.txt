add_executable( test_config test_config.cc )
target_link_libraries( test_config ${CMAKE_PROJECT_NAME}_lib GTest::gtest_main )
gtest_discover_tests( test_config )

add_executable( test_pid_file test_pid_file.cc )
target_link_libraries( test_pid_file ${CMAKE_PROJECT_NAME}_lib GTest::gtest_main )
gtest_discover_tests( test_pid_file )

add_executable( test_executor test_executor.cc )
target_link_libraries( test_executor ${CMAKE_PROJECT_NAME}_lib GTest::gtest_main )
gtest_discover_tests( test_executor )

add_executable( test_data_dir test_data_dir.cc )
target_link_libraries( test_data_dir ${CMAKE_PROJECT_NAME}_lib GTest::gtest_main )
gtest_discover_tests( test_data_dir )

add_executable( test_database test_database.cc )
target_link_libraries( test_database ${CMAKE_PROJECT_NAME}_lib GTest::gtest_main )
gtest_discover_tests( test_database )

add_executable( test_process test_process.cc )
target_link_libraries( test_process ${CMAKE_PROJECT_NAME}_lib GTest::gtest_main )
gtest_discover_tests( test_process )

add_executable( test_log_manager test_log_manager.cc )
target_link_libraries( test_log_manager ${CMAKE_PROJECT_NAME}_lib GTest::gtest_main )
gtest_discover_tests( test_log_manager )

if(NOT WIN32)
    add_executable( test_daemonize test_daemonize.cc )
    target_link_libraries( test_daemonize ${CMAKE_PROJECT_NAME}_lib GTest::gtest_main )
    gtest_discover_tests( test_daemonize )
endif()

# Integration tests (shell-based)
if(WIN32)
    add_test(NAME integration_tests
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/tests/test_frequent_cron.ps1 -Binary ${CMAKE_SOURCE_DIR}/frequent-cron.exe
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
else()
    add_test(NAME integration_tests
        COMMAND bash ${CMAKE_SOURCE_DIR}/tests/test_frequent_cron.sh ${CMAKE_SOURCE_DIR}/frequent-cron
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
