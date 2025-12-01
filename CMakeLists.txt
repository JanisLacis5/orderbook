cmake_minimum_required(VERSION 3.28)
project(orderbook)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
