# Library (everything except main)
add_library( ${CMAKE_PROJECT_NAME}_lib STATIC
    config.cc
    executor.cc
    pid_file.cc
    daemonize.cc
    data_dir.cc
    database.cc
    service_registry.cc
    platform_service.cc
    process.cc
    log_manager.cc
)

find_package( Boost 1.37 COMPONENTS program_options REQUIRED )
find_package( SQLite3 REQUIRED )
find_package( Threads REQUIRED )
target_include_directories( ${CMAKE_PROJECT_NAME}_lib PUBLIC ${CMAKE_SOURCE_DIR}/include ${Boost_INCLUDE_DIR} )
target_link_libraries( ${CMAKE_PROJECT_NAME}_lib PUBLIC ${Boost_LIBRARIES} SQLite::SQLite3 Threads::Threads )

if(WIN32)
    target_compile_definitions( ${CMAKE_PROJECT_NAME}_lib PUBLIC _WIN32_WINNT=0x0601 NOMINMAX WIN32_LEAN_AND_MEAN )
    # Force include <process.h> on Windows -- WIN32_LEAN_AND_MEAN strips it from <windows.h>
    # but Boost ASIO and pid_file need _beginthreadex/_getpid from it
    target_compile_options( ${CMAKE_PROJECT_NAME}_lib PUBLIC /FI"process.h" )
endif()

# Executable
add_executable( ${CMAKE_PROJECT_NAME} main.cc )
target_link_libraries( ${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_NAME}_lib )

set_target_properties( ${CMAKE_PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}
)

install( TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin )

# Set default compile flags for GCC
if(CMAKE_COMPILER_IS_GNUCXX)
  message(STATUS "GCC detected, adding compile flags")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif(CMAKE_COMPILER_IS_GNUCXX)
