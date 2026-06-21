add_library(obBench
    bench.cpp
)
target_include_directories(obBench
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src/tools/obBench
)
target_link_libraries(obBench
    PUBLIC common commandParser orderbook
)

add_executable(orderbook_benchmark main.cpp)
target_link_libraries(orderbook_benchmark
    PRIVATE obBench
)
