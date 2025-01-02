cmake_minimum_required(VERSION 3.10)

# Project Configuration
project(amemorylibrary VERSION 0.1.1)

# Variables
set(INCLUDE_DIR_NAME "a-memory-library")
set(EXTRA_FILES README.md AUTHORS NEWS.md CHANGELOG.md LICENSE NOTICE)
set(INSTALL_LIBDIR lib)
set(INSTALL_INCLUDEDIR include)
set(INSTALL_BINDIR bin)
set(INSTALL_DOCDIR share/doc/${PROJECT_NAME})

# Source files
file(GLOB SOURCE_FILES src/*.c)

# Dependencies
find_package(themacrolibrary REQUIRED)

# Options
option(DEBUG "Enable debugging" OFF)
option(ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BUILD_DEBUG_LIBS "Build debug libraries" ON)
option(BUILD_TESTING "Build tests" OFF)
option(ENABLE_CODE_COVERAGE "Enable code coverage reporting" OFF)
option(ENABLE_CLANG_TIDY "Enable Clang-Tidy analysis" OFF)

# C version
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler options
if(ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address)
endif()

if(ENABLE_CLANG_TIDY)
    set(CMAKE_C_CLANG_TIDY clang-tidy)
endif()

if(ENABLE_CODE_COVERAGE)
    add_compile_options(-fprofile-arcs -ftest-coverage)
    set(COVERAGE_LIBS -fprofile-arcs -ftest-coverage)
endif()

# Debug library
if(BUILD_DEBUG_LIBS)
    add_library(${PROJECT_NAME}_debug STATIC ${SOURCE_FILES})
    target_include_directories(${PROJECT_NAME}_debug PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDEDIR}>)
    target_compile_definitions(${PROJECT_NAME}_debug PUBLIC -D_AML_DEBUG_)
    target_compile_options(${PROJECT_NAME}_debug PRIVATE -g -O0)
    set_target_properties(${PROJECT_NAME}_debug PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_debug")
    if(ENABLE_CODE_COVERAGE)
        target_link_libraries(${PROJECT_NAME}_debug PRIVATE ${COVERAGE_LIBS})
    endif()
    add_library(${PROJECT_NAME}::debug ALIAS ${PROJECT_NAME}_debug)
endif()

# Static library
if(BUILD_STATIC_LIBS)
    add_library(${PROJECT_NAME}_static STATIC ${SOURCE_FILES})
    target_include_directories(${PROJECT_NAME}_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDEDIR}>)
    target_compile_options(${PROJECT_NAME}_static PRIVATE -O3)
    set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_static")
    if(ENABLE_CODE_COVERAGE)
        target_link_libraries(${PROJECT_NAME}_static PRIVATE ${COVERAGE_LIBS})
    endif()
    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME}_static)
    add_library(${PROJECT_NAME}::default ALIAS ${PROJECT_NAME}_static)
endif()

# Shared library
if(BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDEDIR}>)
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
    if(ENABLE_CODE_COVERAGE)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${COVERAGE_LIBS})
    endif()
    add_library(${PROJECT_NAME}::shared ALIAS ${PROJECT_NAME})
endif()

# Installation of the libraries
if(BUILD_STATIC_LIBS)
    install(TARGETS ${PROJECT_NAME}_static
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDEDIR})
endif()

if(BUILD_DEBUG_LIBS)
    install(TARGETS ${PROJECT_NAME}_debug
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDEDIR})
endif()

if(BUILD_SHARED_LIBS)
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${INSTALL_LIBDIR}
        RUNTIME DESTINATION ${INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDEDIR})
endif()

# Export the targets
install(EXPORT ${PROJECT_NAME}Targets
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION ${INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Export the targets
install(EXPORT ${PROJECT_NAME}Targets
        NAMESPACE ${PROJECT_NAME}::
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Generate Config.cmake.in dynamically
file(WRITE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake.in" "
@PACKAGE_INIT@

# Include directory
set(${PROJECT_NAME}_INCLUDE_DIR \"@PACKAGE_INCLUDE_DIR@\")

# Include the exported targets
include(\"${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake\")
")

# Generate cmake_uninstall.cmake.in dynamically
file(WRITE "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake.in" "
if(NOT EXISTS \"@CMAKE_CURRENT_BINARY_DIR@/install_manifest.txt\")
    message(FATAL_ERROR \"Cannot find install manifest: @CMAKE_CURRENT_BINARY_DIR@/install_manifest.txt\")
endif()

file(READ \"@CMAKE_CURRENT_BINARY_DIR@/install_manifest.txt\" files)
string(REGEX REPLACE \"\\n\" \";\" files \"\${files}\")
foreach(file \${files})
    message(STATUS \"Uninstalling \$ENV{DESTDIR}\${file}\")
    if(EXISTS \"\$ENV{DESTDIR}\${file}\")
        execute_process(COMMAND @CMAKE_COMMAND@ -E remove \"\$ENV{DESTDIR}\${file}\")
    else()
        message(STATUS \"File \$ENV{DESTDIR}\${file} does not exist.\")
    endif()
endforeach()
")

# Create and install the config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_file("${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Uninstall command
configure_file("${CMAKE_BINARY_DIR}/cmake_uninstall.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Testing
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_BINDIR})
