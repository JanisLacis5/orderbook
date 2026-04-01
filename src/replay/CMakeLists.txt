add_library(replay
    replay.cpp
)
target_include_directories(replay
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src/replay
)
target_link_libraries(replay
    PUBLIC fileops orderbook common
)
