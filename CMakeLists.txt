cmake_minimum_required(VERSION "3.5")
set(MAIN_EXECUTABLE frequent-cron)

project(${MAIN_EXECUTABLE} CXX C)
set(CMAKE_CXX_STANDARD 23)
add_subdirectory(src)

enable_testing()
add_test(NAME integration_tests
    COMMAND bash ${CMAKE_SOURCE_DIR}/tests/test_frequent_cron.sh ${CMAKE_SOURCE_DIR}/frequent-cron
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
