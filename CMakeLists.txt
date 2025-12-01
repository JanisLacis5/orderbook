cmake_minimum_required(VERSION 3.28)
project(orderbook)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GTEST_GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG ${GTEST_GIT_TAG}
)
FetchContent_MakeAvailable(googletest)

add_library(core
    src/orderbook.cpp
)

target_include_directories(core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

add_executable(orderbook
    src/main.cpp
)

target_link_libraries(orderbook
    PRIVATE core
)

enable_testing()

add_executable(test_core
    tests/unit/test_order.cpp
)
target_link_libraries(test_core
    PRIVATE
        core
        gtest_main
)
include(GoogleTest)
gtest_discover_tests(test_core)
