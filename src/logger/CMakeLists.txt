add_library(logger STATIC
    Logger.cpp
)

target_include_directories(logger
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_options(logger PRIVATE
  -Wall -Wextra -Wpedantic
)

