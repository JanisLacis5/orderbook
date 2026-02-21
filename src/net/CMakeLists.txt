add_library(net STATIC
    EpollManager.cpp
)

target_include_directories(net
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(net
    PUBLIC logger
)

target_compile_options(net PRIVATE
  -Wall -Wextra -Wpedantic
)

