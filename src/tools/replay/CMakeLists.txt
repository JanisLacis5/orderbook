add_library(replaylib
    replay.cpp
)
target_include_directories(replaylib
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src/tools/replay
)
target_link_libraries(replaylib
    PUBLIC orderbook common logger commandParser
)

add_executable(replay main.cpp)
target_link_libraries(replay 
    PRIVATE replaylib
)
