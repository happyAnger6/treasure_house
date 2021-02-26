set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

aux_source_directory(memory SRCS)
aux_source_directory(container SRCS)

add_library(treasure_house PRIVATE ${SRCS})

target_include_directories(treasure_house
                        PRIVATE ${PUBLIC_INCLUDE_DIR})