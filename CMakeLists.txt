cmake_minimum_required(VERSION 4.0)
project(orderbook)

set(CMAKE_CXX_STANDARD 26)

add_executable(orderbook main.cpp
        orderbook.cpp
        orderbook.h
        types.h
        usings.h
        trade.h)
