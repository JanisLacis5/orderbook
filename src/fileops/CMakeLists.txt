add_library(fileops
    file.cpp
)
target_include_directories(fileops
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src/fileops
)
