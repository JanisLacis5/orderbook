add_library(api
    PublicAPI.cpp
)
target_include_directories(api
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src/api
)
target_link_libraries(api
    PUBLIC spsc_queue common net user
)


add_library(user
    ${PROJECT_SOURCE_DIR}/src/api/User.cpp
)
target_include_directories(user
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src/api
)
target_link_libraries(user
    PUBLIC net spsc_queue containers common
)
