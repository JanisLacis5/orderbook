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

add_library(producer
    ${PROJECT_SOURCE_DIR}/src/api/Producer.cpp
)
target_include_directories(producer
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src/api
)
target_link_libraries(producer
    PUBLIC orderbook 
)

add_library(consumer
    ${PROJECT_SOURCE_DIR}/src/api/Consumer.cpp
)
target_include_directories(consumer
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src/api
)
target_link_libraries(consumer
    PUBLIC orderbook 
)
