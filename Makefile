# Makefile to simplify CMake commands

# Default build type
# Values: Debug, Release, RelWithDebInfo, MinSizeRel
BUILD_TYPE ?= Debug

# Validate build type according to the possible values
# ifeq ($(filter $(BUILD_TYPE),Debug Release RelWithDebInfo MinSizeRel),)
# 	$(error Invalid build type "$(BUILD_TYPE)")
# endif

# Build directory
BUILD_DIR := build

TARGET_DIR := $(BUILD_DIR)/$(BUILD_TYPE)

.PHONY: all clean build run

build: $(TARGET_DIR)
	echo $(TARGET_DIR)
	cmake --build $(TARGET_DIR)

# Default target: build the project
all:
	$(MAKE) BUILD_TYPE=Release build
	$(MAKE) BUILD_TYPE=Debug build

debug: $(BUILD_DIR)/Debug
	gdb $(BUILD_DIR)/Debug/JNITest

# Create the build directory and run CMake
$(TARGET_DIR): CMakeLists.txt
	cmake -B $(TARGET_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

# Clean up the build directory
clean:
	rm -rf $(TARGET_DIR)

run:
	@cmake  --build $(TARGET_DIR) --target run --quiet
