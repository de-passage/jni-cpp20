cmake_minimum_required(VERSION 3.10)

project(JNI_CPP20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(JNI_CPP20 INTERFACE)
target_include_directories(JNI_CPP20 INTERFACE include)

# Include tests conditionally
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Configure JNI
find_package(JNI REQUIRED)

if (JNI_FOUND)
  message(STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
  message(STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")
endif()

target_include_directories(JNI_CPP20 INTERFACE ${JNI_INCLUDE_DIRS})
target_link_libraries(JNI_CPP20 INTERFACE ${JNI_LIBRARIES})

# Add target to run the binary
add_custom_target(run
  COMMAND JNI_CPP20
  DEPENDS JNI_CPP20
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

