add_library(spsc_queue INTERFACE)
target_include_directories(spsc_queue
    INTERFACE
    ${PROJECT_SOURCE_DIR}/src/data_structures/spsc_queue
)
