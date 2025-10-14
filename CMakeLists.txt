# LS-Game-DataGen - Lot Sizing Game Data Generator - Main CMake Configuration
# Single CMakeLists.txt manages the entire project

# Specify minimum required CMake version
cmake_minimum_required(VERSION 3.24)

# Define project name, version, and languages used
project(LSGameDataGen
    VERSION 1.0.0
    DESCRIPTION "Lot Sizing Game Data Generator"
    LANGUAGES CXX
)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set output directories for multi-configuration generators (e.g., Visual Studio)
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

# Set source file directory
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# Add include directories
include_directories(
    ${SRC_DIR}
)

# Generate compile_commands.json for better IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set runtime library for MSVC compiler
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

# Define source files
set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/case_generator.cpp
    ${SRC_DIR}/csv_writer.cpp
)

# Define header files (for IDE display)
set(HEADERS
    ${SRC_DIR}/case_generator.h
    ${SRC_DIR}/csv_writer.h
    ${SRC_DIR}/logger.h
    ${SRC_DIR}/demand_generator.h
    ${SRC_DIR}/demand_generator_v2.h
)

# Define source files for v2 (using new generator)
set(SOURCES_V2
    ${SRC_DIR}/main_v2.cpp
    ${SRC_DIR}/case_generator.cpp
    ${SRC_DIR}/csv_writer.cpp
)

# Force all files to be at the same level in IDE
source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

# Create executable for v1
add_executable(LSGameDataGen ${SOURCES} ${HEADERS})

# Create executable for v2
add_executable(LSGameDataGenV2 ${SOURCES_V2} ${HEADERS})

# Set target properties
set_target_properties(LSGameDataGen PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)

set_target_properties(LSGameDataGenV2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)

# Add compile definitions
target_compile_definitions(LSGameDataGen PRIVATE
    DATAGEN_VERSION="1.0"
    DATAGEN_VERSION_MAJOR=1
    DATAGEN_VERSION_MINOR=0
)

target_compile_definitions(LSGameDataGenV2 PRIVATE
    DATAGEN_VERSION="2.0"
    DATAGEN_VERSION_MAJOR=2
    DATAGEN_VERSION_MINOR=0
)

# Installation rules
install(TARGETS LSGameDataGen LSGameDataGenV2
    RUNTIME DESTINATION bin
)

# Create run target
add_custom_target(run
    COMMAND LSGameDataGen
    DEPENDS LSGameDataGen
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running LS-Game-DataGen..."
)

# Create test target
enable_testing()
add_test(NAME DataGen_Test
    COMMAND LSGameDataGen
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Generate configuration summary
message(STATUS "")
message(STATUS "=== LS-Game-DataGen Build Configuration ===")
message(STATUS "CMake Version: ${CMAKE_VERSION}")
message(STATUS "Project Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Source Directory: ${SRC_DIR}")
message(STATUS "Output Directory: ${CMAKE_BINARY_DIR}")
message(STATUS "==========================================")
message(STATUS "")

# Provide useful targets for development
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/lib
    COMMENT "Cleaning all build outputs..."
)
