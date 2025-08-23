# SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
# SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
# SPDX-License-Identifier: Apache-2.0

# CMakeLists.txt for tests
cmake_minimum_required(VERSION 3.20)

project(a_memory_library_tests LANGUAGES C)

set(A_BUILD_VARIANT "debug" CACHE STRING
    "Variant to link via a_memory_library::a_memory_library (debug|memory|static|shared)")
set_property(CACHE A_BUILD_VARIANT PROPERTY STRINGS debug memory static shared)

option(A_ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

find_library(M_LIB m)

# ---- Test executables ----
set(TEST_EXECUTABLES "")
add_executable(test_aml_alloc  src/test_aml_alloc.c)

list(APPEND TEST_EXECUTABLES test_aml_alloc)

set_target_properties(test_aml_alloc PROPERTIES
  C_STANDARD 17
  C_STANDARD_REQUIRED YES
)
if("CXX" IN_LIST CMAKE_PROJECT_LANGUAGES)
  set_target_properties(test_aml_alloc PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
  )
endif()

if(NOT TARGET a_memory_library::a_memory_library)
  find_package(a_memory_library CONFIG REQUIRED)
endif()
target_link_libraries(test_aml_alloc PRIVATE a_memory_library::a_memory_library)

if(M_LIB)
  target_link_libraries(test_aml_alloc PRIVATE ${M_LIB})
endif()

if(MSVC)
  target_compile_options(test_aml_alloc PRIVATE /W4)
else()
  target_compile_options(test_aml_alloc PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(A_ENABLE_COVERAGE)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(test_aml_alloc PRIVATE -O0 -g -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(test_aml_alloc PRIVATE -fprofile-instr-generate -fcoverage-mapping)
  elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(test_aml_alloc PRIVATE -O0 -g --coverage)
    target_link_options(test_aml_alloc PRIVATE --coverage)
  endif()
endif()

add_test(NAME test_aml_alloc COMMAND $<TARGET_FILE:test_aml_alloc>)
add_executable(test_aml_buffer  src/test_aml_buffer.c)

list(APPEND TEST_EXECUTABLES test_aml_buffer)

set_target_properties(test_aml_buffer PROPERTIES
  C_STANDARD 17
  C_STANDARD_REQUIRED YES
)
if("CXX" IN_LIST CMAKE_PROJECT_LANGUAGES)
  set_target_properties(test_aml_buffer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
  )
endif()

if(NOT TARGET a_memory_library::a_memory_library)
  find_package(a_memory_library CONFIG REQUIRED)
endif()
target_link_libraries(test_aml_buffer PRIVATE a_memory_library::a_memory_library)

if(M_LIB)
  target_link_libraries(test_aml_buffer PRIVATE ${M_LIB})
endif()

if(MSVC)
  target_compile_options(test_aml_buffer PRIVATE /W4)
else()
  target_compile_options(test_aml_buffer PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(A_ENABLE_COVERAGE)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(test_aml_buffer PRIVATE -O0 -g -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(test_aml_buffer PRIVATE -fprofile-instr-generate -fcoverage-mapping)
  elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(test_aml_buffer PRIVATE -O0 -g --coverage)
    target_link_options(test_aml_buffer PRIVATE --coverage)
  endif()
endif()

add_test(NAME test_aml_buffer COMMAND $<TARGET_FILE:test_aml_buffer>)
add_executable(test_aml_pool  src/test_aml_pool.c)

list(APPEND TEST_EXECUTABLES test_aml_pool)

set_target_properties(test_aml_pool PROPERTIES
  C_STANDARD 17
  C_STANDARD_REQUIRED YES
)
if("CXX" IN_LIST CMAKE_PROJECT_LANGUAGES)
  set_target_properties(test_aml_pool PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
  )
endif()

if(NOT TARGET a_memory_library::a_memory_library)
  find_package(a_memory_library CONFIG REQUIRED)
endif()
target_link_libraries(test_aml_pool PRIVATE a_memory_library::a_memory_library)

if(M_LIB)
  target_link_libraries(test_aml_pool PRIVATE ${M_LIB})
endif()

if(MSVC)
  target_compile_options(test_aml_pool PRIVATE /W4)
else()
  target_compile_options(test_aml_pool PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(A_ENABLE_COVERAGE)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(test_aml_pool PRIVATE -O0 -g -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(test_aml_pool PRIVATE -fprofile-instr-generate -fcoverage-mapping)
  elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(test_aml_pool PRIVATE -O0 -g --coverage)
    target_link_options(test_aml_pool PRIVATE --coverage)
  endif()
endif()

add_test(NAME test_aml_pool COMMAND $<TARGET_FILE:test_aml_pool>)

enable_testing()

# ---- Coverage aggregation ----
add_custom_target(coverage_report COMMENT "Generate coverage report")

if(A_ENABLE_COVERAGE AND CMAKE_C_COMPILER_ID MATCHES "Clang")
  include(ProcessorCount)
  ProcessorCount(NPROC)
  if(NPROC EQUAL 0)
    set(NPROC 4)
  endif()

  add_custom_command(TARGET coverage_report PRE_BUILD
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -j ${NPROC}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running tests with coverage instrumentation"
  )

  find_program(LLVM_PROFDATA_EXECUTABLE llvm-profdata)
  find_program(LLVM_COV_EXECUTABLE llvm-cov)
  if(LLVM_PROFDATA_EXECUTABLE AND LLVM_COV_EXECUTABLE)
    add_custom_command(TARGET coverage_report POST_BUILD
      COMMAND ${LLVM_PROFDATA_EXECUTABLE} merge -sparse *.profraw -o default.profdata
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Merging coverage data"
    )

    # Normal compiled library path
    add_custom_command(TARGET coverage_report POST_BUILD
      COMMAND ${LLVM_COV_EXECUTABLE} show
              ../liba_memory_library_${A_BUILD_VARIANT}.a
              -instr-profile=default.profdata
              -format=html -output-dir=coverage_html
              --ignore-filename-regex='/usr/.*'
              --show-regions --show-line-counts-or-regions
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generated LLVM coverage report → open coverage_html/index.html"
    )
  endif()
endif()
