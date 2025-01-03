cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

# Define the library to test
set(LIB_TO_TEST amemorylibrary)
file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

find_package(a-cmake-library REQUIRED)
include(BinaryConfig)
