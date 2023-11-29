cmake_minimum_required(VERSION 3.10)
project(amemorylibrary VERSION 0.1.1)

set(CMAKE_INSTALL_DOCDIR share/doc/amemorylibrary)
set(CMAKE_INSTALL_INCLUDEDIR include)

# Options
option(DEBUG "Enable debugging" OFF)
option(ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)

# Set C standard
set(CMAKE_C_STANDARD 99)

# Compiler options
if(DEBUG)
    add_compile_options(-g -O0)
else()
    add_compile_options(-O3)
endif()

if(ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address)
endif()

# Source files
set(SOURCE_FILES src/aml_alloc.c src/aml_buffer.c src/aml_pool.c)

# Shared library
add_library(amemorylibrary SHARED ${SOURCE_FILES})
target_compile_options(amemorylibrary PRIVATE -O3)
target_include_directories(amemorylibrary PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
set_target_properties(amemorylibrary PROPERTIES
    PUBLIC_HEADER "include/a-memory-library/aml_alloc.h;include/a-memory-library/aml_buffer.h;include/a-memory-library/aml_pool.h"
    OUTPUT_NAME "amemorylibrary")

# Debug library
add_library(amemorylibrary_debug STATIC ${SOURCE_FILES})
target_compile_definitions(amemorylibrary_debug PUBLIC -D_AML_DEBUG_)
target_compile_options(amemorylibrary_debug PRIVATE -g -O0)
set_target_properties(amemorylibrary_debug PROPERTIES OUTPUT_NAME "amemorylibrary_debug")
target_include_directories(amemorylibrary_debug PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Release library
add_library(amemorylibrary_static STATIC ${SOURCE_FILES})
set_target_properties(amemorylibrary_static PROPERTIES OUTPUT_NAME "amemorylibrary_static")
target_compile_options(amemorylibrary_static PRIVATE -O3)
target_include_directories(amemorylibrary_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Install the library
install(TARGETS amemorylibrary amemorylibrary_debug amemorylibrary_static
        EXPORT amemorylibraryTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/a-memory-library)

# Export the targets
install(EXPORT amemorylibraryTargets
        FILE amemorylibraryTargets.cmake
        DESTINATION lib/cmake/amemorylibrary)

# Create and install the config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "amemorylibraryConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_file(amemorylibraryConfig.cmake.in amemorylibraryConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/amemorylibraryConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/amemorylibraryConfigVersion.cmake"
        DESTINATION lib/cmake/amemorylibrary)

# Installation of other files
install(FILES README.md AUTHORS NEWS.md CHANGELOG.md LICENSE NOTICE
        DESTINATION ${CMAKE_INSTALL_DOCDIR})

# Uninstall command
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/bin/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
