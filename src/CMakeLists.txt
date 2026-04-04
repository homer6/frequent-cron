file(GLOB sources *.cc)
add_executable( ${CMAKE_PROJECT_NAME} ${sources} )

set_target_properties( ${CMAKE_PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )

find_package( Boost 1.37 COMPONENTS program_options REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )
target_link_libraries( ${CMAKE_PROJECT_NAME} ${Boost_LIBRARIES} )

install( TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin )


# Set default compile flags for GCC
if(CMAKE_COMPILER_IS_GNUCXX)
  message(STATUS "GCC detected, adding compile flags")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif(CMAKE_COMPILER_IS_GNUCXX)
