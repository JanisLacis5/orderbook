add_library(commandParser
    commandParser.cpp
)
target_include_directories(commandParser
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src/tools/commandParser
)
target_link_libraries(commandParser
    PUBLIC common logger
)
