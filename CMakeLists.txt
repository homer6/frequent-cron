cmake_minimum_required(VERSION "2.8.2")
set(MAIN_EXECUTABLE frequent-cron)

project(${MAIN_EXECUTABLE} CXX C)
add_subdirectory(src)
include_directories(lib)




#add_subdirectory(tests EXCLUDE_FROM_ALL)
#set(PROJECT_TESTS ${CMAKE_PROJECT_NAME}_tests)
#enable_testing()
#add_test(tests/${PROJECT_TESTS} tests/${PROJECT_TESTS})
#add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
#add_dependencies(check  ${PROJECT_TESTS})
