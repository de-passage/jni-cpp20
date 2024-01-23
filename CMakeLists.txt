cmake_minimum_required(VERSION 3.10)

project(JNITest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(JNITest src/main.cpp)

# compile_commands.json
#
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_custom_command(TARGET JNITest
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
  ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)

# Configure JNO
#
find_package(JNI REQUIRED)

if (JNI_FOUND)
  message(STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
  message(STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")
endif()

target_include_directories(JNITest PRIVATE ${JNI_INCLUDE_DIRS})
target_link_libraries(JNITest PRIVATE ${JNI_LIBRARIES})

# Add target to run the binary
add_custom_target(run
  COMMAND JNITest
  DEPENDS JNITest
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

