# Makefile to simplify CMake commands
-include Makefile.local

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

CMAKE ?= cmake

.PHONY: all clean build run

build: $(TARGET_DIR)
	$(CMAKE) --build $(TARGET_DIR)

# Default target: build the project
all:
	$(MAKE) BUILD_TYPE=Release build
	$(MAKE) BUILD_TYPE=Debug build

debug: $(BUILD_DIR)/Debug
	gdb $(BUILD_DIR)/Debug/JNITest

# Create the build directory and run CMake
$(TARGET_DIR): CMakeLists.txt
	$(CMAKE) -B $(TARGET_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@ln -s $(TARGET_DIR)/compile_commands.json compile_commands.json 2>/dev/null || true

# Clean up the build directory
clean:
	rm -rf $(BUILD_DIR) 2>/dev/null

run: $(TARGET_DIR)
	$(CMAKE) --build $(TARGET_DIR)  --target run -- --quiet
