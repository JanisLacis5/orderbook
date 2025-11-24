cmake_minimum_required(VERSION 3.0)
project(orderbook)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(orderbook src/main.cpp
        src/orderbook.cpp
        src/orderbook.h
        src/types.h
        src/usings.h
        src/trade.h)
