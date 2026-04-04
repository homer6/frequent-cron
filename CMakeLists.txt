cmake_minimum_required(VERSION "3.5")
set(MAIN_EXECUTABLE frequent-cron)

project(${MAIN_EXECUTABLE} CXX C)
set(CMAKE_CXX_STANDARD 23)
add_subdirectory(src)

enable_testing()
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
