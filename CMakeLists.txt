cmake_minimum_required(VERSION 3.12)

# Project information
project(NEXUS 
    VERSION 1.0.0
    DESCRIPTION "ML-First Programming Language with Java/C# Syntax and C++ Performance"
    LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
endif()

# Options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_DOCS "Build documentation" OFF)
option(ENABLE_GPU "Enable GPU acceleration" OFF)
option(ENABLE_BLAS "Enable BLAS acceleration" OFF)
option(ENABLE_PROFILING "Enable profiling support" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Find packages
find_package(Threads REQUIRED)

# Optional dependencies
if(ENABLE_GPU)
    find_package(CUDA REQUIRED)
    find_package(CUDAToolkit REQUIRED)
    add_definitions(-DUSE_GPU)
endif()

if(ENABLE_BLAS)
    find_package(BLAS REQUIRED)
    find_package(LAPACK REQUIRED)
    add_definitions(-DUSE_BLAS)
endif()

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
endif()

if(BUILD_DOCS)
    find_package(Doxygen REQUIRED)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Source files
set(NEXUS_SOURCES
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
    src/value.cpp
    src/environment.cpp
    src/ml/neural_network.cpp
    src/ml/tensor.cpp
    src/ml/layers.cpp
    src/ml/optimizers.cpp
    src/utils/file_utils.cpp
    src/utils/math_utils.cpp
)

# Header files
set(NEXUS_HEADERS
    include/nexus/nexus.h
    include/nexus/config.h
    include/nexus/version.h
    src/lexer.h
    src/parser.h
    src/interpreter.h
    src/value.h
    src/environment.h
    src/ml/neural_network.h
    src/ml/tensor.h
    src/ml/layers.h
    src/ml/optimizers.h
    src/utils/file_utils.h
    src/utils/math_utils.h
)

# Create main executable
add_executable(nexus ${NEXUS_SOURCES} ${NEXUS_HEADERS})

# Link libraries
target_link_libraries(nexus Threads::Threads)

if(UNIX AND NOT APPLE)
    target_link_libraries(nexus dl)
elseif(APPLE)
    find_library(ACCELERATE_FRAMEWORK Accelerate)
    target_link_libraries(nexus ${ACCELERATE_FRAMEWORK})
endif()

if(ENABLE_GPU)
    target_link_libraries(nexus CUDA::cuda_runtime CUDA::cublas CUDA::cudnn)
endif()

if(ENABLE_BLAS)
    target_link_libraries(nexus ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
endif()

# Set target properties
set_target_properties(nexus PROPERTIES
    OUTPUT_NAME nexus
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Platform-specific settings
if(WIN32)
    set_target_properties(nexus PROPERTIES OUTPUT_NAME nexus.exe)
endif()

# Profiling support
if(ENABLE_PROFILING)
    target_compile_options(nexus PRIVATE -pg -fprofile-arcs -ftest-coverage)
    target_link_options(nexus PRIVATE -pg -fprofile-arcs -ftest-coverage)
    target_link_libraries(nexus gcov)
endif()

# Tests
if(BUILD_TESTS)
    # Test sources
    file(GLOB TEST_SOURCES 
        tests/unit/*.cpp
        tests/integration/*.cpp
    )
    
    # Create test executable
    add_executable(nexus_tests ${TEST_SOURCES})
    
    # Link test libraries
    target_link_libraries(nexus_tests 
        GTest::gtest 
        GTest::gtest_main
        Threads::Threads
    )
    
    # Include main source files (except main.cpp)
    list(REMOVE_ITEM NEXUS_SOURCES src/main.cpp)
    target_sources(nexus_tests PRIVATE ${NEXUS_SOURCES})
    
    # Set test properties
    set_target_properties(nexus_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    # Register tests
    add_test(NAME NexusUnitTests COMMAND nexus_tests)
    
    # Integration tests
    if(EXISTS ${CMAKE_SOURCE_DIR}/examples)
        file(GLOB EXAMPLE_FILES ${CMAKE_SOURCE_DIR}/examples/basic/*.nx)
        foreach(example_file ${EXAMPLE_FILES})
            get_filename_component(example_name ${example_file} NAME_WE)
            add_test(
                NAME IntegrationTest_${example_name}
                COMMAND nexus ${example_file}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            )
        endforeach()
    endif()
endif()

# Examples
if(BUILD_EXAMPLES)
    # Copy examples to build directory
    file(COPY ${CMAKE_SOURCE_DIR}/examples 
         DESTINATION ${CMAKE_BINARY_DIR})
    
    # Copy standard library
    file(COPY ${CMAKE_SOURCE_DIR}/stdlib 
         DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Documentation
if(BUILD_DOCS AND DOXYGEN_FOUND)
    # Configure Doxyfile
    set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)
    
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    # Add documentation target
    add_custom_target(docs ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM
    )
endif()

# Installation
include(GNUInstallDirs)

# Install binary
install(TARGETS nexus
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Install standard library
install(DIRECTORY ${CMAKE_SOURCE_DIR}/stdlib/
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/nexus
)

# Install examples
if(BUILD_EXAMPLES)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/examples/
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples
    )
endif()

# Install documentation
if(BUILD_DOCS)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/docs/html/
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        OPTIONAL
    )
endif()

# Package configuration
include(CMakePackageConfigHelpers)

# Create package config file
configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/NEXUSConfig.cmake.in
    ${CMAKE_BINARY_DIR}/NEXUSConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NEXUS
)

# Create version file
write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/NEXUSConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install package files
install(FILES
    ${CMAKE_BINARY_DIR}/NEXUSConfig.cmake
    ${CMAKE_BINARY_DIR}/NEXUSConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NEXUS
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "NEXUS")
set(CPACK_PACKAGE_VENDOR "NEXUS Language Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "NEXUS")

# Platform-specific package settings
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "NEXUS Programming Language")
    set(CPACK_NSIS_HELP_LINK "https://nexus-lang.github.io/nexus")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/nexus-lang/nexus")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    
    # Debian package settings
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "NEXUS Team <support@nexus-lang.org>")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6, libgcc1")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    
    # RPM package settings
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
    set(CPACK_RPM_PACKAGE_REQUIRES "glibc, libstdc++, libgcc")
endif()

include(CPack)

# Custom targets
add_custom_target(format
    COMMAND clang-format -i ${NEXUS_SOURCES} ${NEXUS_HEADERS}
    COMMENT "Formatting source code"
)

add_custom_target(lint
    COMMAND cpplint ${NEXUS_SOURCES} ${NEXUS_HEADERS}
    COMMENT "Linting source code"
)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/docs
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/coverage
    COMMENT "Cleaning all build artifacts"
)

# Coverage target
if(ENABLE_COVERAGE)
    add_custom_target(coverage
        COMMAND lcov --capture --directory ${CMAKE_BINARY_DIR} --output-file coverage.info
        COMMAND genhtml coverage.info --output-directory coverage_html
        COMMENT "Generating code coverage report"
        DEPENDS nexus_tests
    )
endif()

# Benchmark target
add_custom_target(benchmark
    COMMAND ${CMAKE_BINARY_DIR}/bin/nexus ${CMAKE_SOURCE_DIR}/examples/benchmarks/performance_test.nx
    COMMENT "Running performance benchmarks"
    DEPENDS nexus
)

# Memory test target
add_custom_target(memory-test
    COMMAND valgrind --leak-check=full --show-leak-kinds=all ${CMAKE_BINARY_DIR}/bin/nexus ${CMAKE_SOURCE_DIR}/examples/basic/hello_world.nx
    COMMENT "Running memory leak detection"
    DEPENDS nexus
)

# Print configuration summary
message(STATUS "")
message(STATUS "NEXUS Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Features:")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "  Examples: ${BUILD_EXAMPLES}")
message(STATUS "  Documentation: ${BUILD_DOCS}")
message(STATUS "  GPU support: ${ENABLE_GPU}")
message(STATUS "  BLAS acceleration: ${ENABLE_BLAS}")
message(STATUS "  Profiling: ${ENABLE_PROFILING}")
message(STATUS "  Coverage: ${ENABLE_COVERAGE}")
message(STATUS "")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")
message(STATUS "")