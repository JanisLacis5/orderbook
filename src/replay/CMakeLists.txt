add_library(replay
    replay.cpp
)
target_include_directories(replay
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src/replay
)
target_link_libraries(replay
    PUBLIC orderbook common logger
)

add_executable(replayexe main.cpp)
target_link_libraries(replayexe 
    PRIVATE replay
)
