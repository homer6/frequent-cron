set(PROJECT_TESTS ${CMAKE_PROJECT_NAME}_tests)
include_directories(${${CMAKE_PROJECT_NAME}_SOURCE_DIR}/src)

file(GLOB sources *.cc)
add_executable(${PROJECT_TESTS} ${sources})

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

set(libs ${CMAKE_PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries(${PROJECT_TESTS} ${libs})

# Set default compile flags for GCC
if(CMAKE_COMPILER_IS_GNUCXX)
  message(STATUS "GCC detected, adding compile flags")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif(CMAKE_COMPILER_IS_GNUCXX)
