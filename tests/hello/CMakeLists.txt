cmake_minimum_required(VERSION 3.0)
project(HelloWorld LANGUAGES CXX)

add_executable(hello_world cpp/hello.cpp)
target_link_libraries(hello_world PRIVATE JNI_CPP20)

add_test(NAME HelloWorld COMMAND hello_world)

# Set variables for Java files and class output directory
set(JAVA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/java)
set(JAVA_CLASS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/java_classes)

find_package(Java REQUIRED)

# Ensure the class output directory exists
file(MAKE_DIRECTORY ${JAVA_CLASS_OUTPUT_DIR})

# Compile Java source file(s) to the class output directory
add_custom_command(
  OUTPUT ${JAVA_CLASS_OUTPUT_DIR}/Hello.class
  COMMAND ${Java_JAVAC_EXECUTABLE} -d ${JAVA_CLASS_OUTPUT_DIR} ${JAVA_SOURCE_DIR}/Hello.java
  DEPENDS ${JAVA_SOURCE_DIR}/Hello.java
  COMMENT "Compiling Hello.java"
)

# Create a custom target for the Java compilation
add_custom_target(CompileJava ALL
  DEPENDS ${JAVA_CLASS_OUTPUT_DIR}/Hello.class
)

target_compile_definitions(hello_world PRIVATE JAVA_CLASSPATH="${JAVA_CLASS_OUTPUT_DIR}")
