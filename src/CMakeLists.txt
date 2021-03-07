set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CONTAINER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/container)

aux_source_directory(memory SRCS)
aux_source_directory(container SRCS)
aux_source_directory(threadpool SRCS)
aux_source_directory(queue SRCS)

add_library(treasure_house STATIC ${SRCS})

target_include_directories(treasure_house
                        PRIVATE ${PUBLIC_INCLUDE_DIR}
                        PRIVATE ${CONTAINER_INCLUDE_DIR}
)

target_link_libraries(treasure_house
                    PRIVATE pthread
)