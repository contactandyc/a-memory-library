cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

set(LIB_TO_TEST amemorylibrary)

# Options
option(USE_STATIC_LIB "Link against static library" OFF)
option(USE_SHARED_LIB "Link against shared library" OFF)
option(ENABLE_CODE_COVERAGE "Enable code coverage reporting" OFF)

# Set variables
set(TEST_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB TEST_SOURCES ${TEST_SOURCES_DIR}/*.c)

# Add a library for code coverage (if required)
if(ENABLE_CODE_COVERAGE)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("--coverage" HAS_COVERAGE_FLAG)
    if(HAS_COVERAGE_FLAG)
        add_compile_options(--coverage)
        link_libraries(--coverage)
    else()
        find_program(LLVM_COV_EXECUTABLE llvm-cov)
        find_program(GCOV_EXECUTABLE gcov)
        if(LLVM_COV_EXECUTABLE)
            add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
            link_libraries(-fprofile-instr-generate -fcoverage-mapping)
        elseif(GCOV_EXECUTABLE)
            add_compile_options(-fprofile-arcs -ftest-coverage)
            link_libraries(-fprofile-arcs -ftest-coverage)

            # Add a custom target to generate gcov reports
            add_custom_target(coverage
                COMMAND ${CMAKE_COMMAND} -E make_directory coverage-report
                COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcno" -execdir ${GCOV_EXECUTABLE} -o . {} \\\; > coverage-report/coverage.txt
                COMMAND cat coverage-report/coverage.txt
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report using gcov")

        else()
            message(WARNING "No suitable code coverage tool found. Code coverage is disabled.")
            set(ENABLE_CODE_COVERAGE OFF)
        endif()
    endif()
endif()

# Include directories for the main library
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(themacrolibrary REQUIRED)

# Determine which library to link against
if(USE_STATIC_LIB)
    set(LIB_TO_LINK ${LIB_TO_TEST}::${LIB_TO_TEST})
elseif(USE_SHARED_LIB)
    set(LIB_TO_LINK ${LIB_TO_TEST}::shared)
else()
    set(LIB_TO_LINK ${LIB_TO_TEST}::debug)
endif()

# Add test executables
foreach(TEST_SOURCE ${TEST_SOURCES})
    # Extract test name (e.g., test_io.c -> test_io)
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    # Add executable for the test
    add_executable(${TEST_NAME} ${TEST_SOURCE})

    if(USE_STATIC_LIB)
        target_compile_options(${TEST_NAME} PRIVATE -O3)
    else()
        target_compile_definitions(${TEST_NAME} PUBLIC -D_AML_DEBUG_)
        target_compile_options(${TEST_NAME} PRIVATE -g -O0)
    endif()

    # Link against the chosen library
    target_link_libraries(${TEST_NAME} PRIVATE ${LIB_TO_LINK})


    # Add coverage flags if enabled
    if(ENABLE_CODE_COVERAGE)
        target_compile_options(${TEST_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_options(${TEST_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
    endif()

    # Register the test with CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# Additional configurations for code coverage tools (e.g., gcov, lcov)
if(ENABLE_CODE_COVERAGE)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)

    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        add_custom_target(coverage
            COMMAND ${LCOV_EXECUTABLE} --capture --directory . --output-file coverage.info
            COMMAND ${LCOV_EXECUTABLE} --remove coverage.info '/usr/*' "${CMAKE_BINARY_DIR}/*" --output-file coverage.info
            COMMAND ${GENHTML_EXECUTABLE} coverage.info --output-directory coverage-report
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report")
    elseif(GCOV_EXECUTABLE)
        message(STATUS "Using gcov for code coverage.")
    else()
        message(WARNING "lcov, genhtml, or gcov not found. Coverage report generation will be unavailable.")
    endif()
endif()
