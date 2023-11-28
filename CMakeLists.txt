cmake_minimum_required(VERSION 3.10)
project(amemorylibrary VERSION 0.1.1)

option(DEBUG "Enable debugging" OFF)
option(ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)

set(CMAKE_INSTALL_INCLUDEDIR include)
set(CMAKE_INSTALL_BINDIR bin)
set(CMAKE_INSTALL_DOCDIR share/doc/amemorylibrary)
set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SOURCE_DIR}/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_SOURCE_DIR}/include")

if(ADDRESS_SANITIZER)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

# Install README, AUTHORS, etc.
install(FILES README.md AUTHORS NEWS.md CHANGELOG.md LICENSE NOTICE DESTINATION ${CMAKE_INSTALL_DOCDIR})

# Install header files
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/a-memory-library DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(CODE "execute_process(COMMAND git log -n1 OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/GIT_REF)")
install(FILES LICENSE NOTICE ${CMAKE_CURRENT_BINARY_DIR}/GIT_REF DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/a-memory-library)

set(SOURCE_FILES src/aml_alloc.c src/aml_buffer.c src/aml_pool.c)

# Debug library
add_library(amemorylibrary_debug STATIC ${SOURCE_FILES})
target_compile_definitions(amemorylibrary_debug PUBLIC -D_AML_DEBUG_)
set_target_properties(amemorylibrary_debug PROPERTIES OUTPUT_NAME "amemorylibrary_debug")
target_compile_options(amemorylibrary_debug PRIVATE -g -O0)

# Release library
add_library(amemorylibrary_static STATIC ${SOURCE_FILES})
set_target_properties(amemorylibrary_static PROPERTIES OUTPUT_NAME "amemorylibrary_static")
target_compile_options(amemorylibrary_static PRIVATE -O3)

# Shared library
add_library(amemorylibrary SHARED ${SOURCE_FILES})
set_target_properties(amemorylibrary PROPERTIES OUTPUT_NAME "amemorylibrary")
target_compile_options(amemorylibrary PRIVATE -O3)

# Export targets
install(TARGETS amemorylibrary amemorylibrary_static amemorylibrary_debug
        EXPORT amemorylibraryTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "amemorylibraryConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(EXPORT amemorylibraryTargets
        FILE amemorylibraryTargets.cmake
        DESTINATION lib/cmake/amemorylibrary)

configure_file(amemorylibraryConfig.cmake.in amemorylibraryConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/amemorylibraryConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/amemorylibraryConfigVersion.cmake"
        DESTINATION lib/cmake/amemorylibrary)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/bin/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
