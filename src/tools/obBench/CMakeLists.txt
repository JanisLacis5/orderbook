add_library(obBench
    bench.cpp
)
target_include_directories(obBench
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src/tools/obBench
)
target_link_libraries(obBench
    PUBLIC common logger commandParser
)

add_executable(orderbook_benchmark bench.cpp)
target_link_libraries(orderbook_benchmark
    PRIVATE obBench
)
