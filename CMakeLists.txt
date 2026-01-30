cmake_minimum_required(VERSION 3.28)
project(orderbook)

# CMAKE VARIABLES
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CUSTOM VARIABLES
set(GTEST_GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723)
set(CLANG_FORMAT_BIN "/usr/bin/clang-format-20")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

# TSAN
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
add_library(project_sanitizers INTERFACE)
if (ENABLE_TSAN)
    target_compile_options(project_sanitizers INTERFACE
        -fsanitize=thread
    )
    target_link_options(project_sanitizers INTERFACE
        -fsanitize=thread
    )
endif ()

# EXTERNAL LIBRARIES
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG ${GTEST_GIT_TAG}
)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG main
)
FetchContent_MakeAvailable(googletest googlebenchmark)

# CORE LIBRARIES / EXECUTABLES
# Data Structures
add_library(data_structures INTERFACE)
target_include_directories(data_structures
    INTERFACE
    ${PROJECT_SOURCE_DIR}/src/data_structures
)

# Common
add_library(common INTERFACE)
target_include_directories(common
    INTERFACE
    ${PROJECT_SOURCE_DIR}/src/common
)

# Orderbook
add_library(orderbook
    ${PROJECT_SOURCE_DIR}/src/orderbook/orderbook.cpp
)
target_include_directories(orderbook
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src/orderbook
)
target_link_libraries(orderbook
    PUBLIC data_structures common
)

# API
add_library(publicAPI
    ${PROJECT_SOURCE_DIR}/src/public_api/PublicAPI.cpp
)
target_include_directories(publicAPI
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src/public_api
)
target_link_libraries(publicAPI
    PUBLIC data_structures common
)

# Main
add_executable(app
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)
target_link_libraries(app
    PRIVATE orderbook publicAPI
)

# FORMATTING TARGET
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/.clang-format")
    message(FATAL_ERROR "No .clang-format file found in ${CMAKE_SOURCE_DIR}")
endif ()

file(GLOB_RECURSE FORMAT_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/tests/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/*.h"
)

add_custom_target(format
    COMMAND ${CLANG_FORMAT_BIN} -i ${FORMAT_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running ${CLANG_FORMAT_BIN} on core + orderbook sources"
)

# TESTING
enable_testing()

add_executable(test_core
    tests/unit/test_order.cpp
    tests/unit/test_orderbook.cpp
    tests/unit/test_spscqueue.cpp
)
target_link_libraries(test_core
    PRIVATE
    common
    orderbook
    data_structures
    gtest_main
    project_sanitizers
)
include(GoogleTest)
gtest_discover_tests(test_core DISCOVERY_MODE PRE_TEST)

# BENCHMARK
if (NOT ENABLE_TSAN)  # TODO: do this better (check if built in release mode)
    add_executable(bench "${PROJECT_SOURCE_DIR}/tests/benchmark/bench.cpp")
    target_link_libraries(bench PRIVATE benchmark::benchmark data_structures)
endif ()
