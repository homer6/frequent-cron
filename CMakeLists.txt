cmake_minimum_required(VERSION "3.14")
set(MAIN_EXECUTABLE frequent-cron)

project(${MAIN_EXECUTABLE} CXX C)
set(CMAKE_CXX_STANDARD 23)
add_subdirectory(src)

# Testing
enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
add_subdirectory(tests)
