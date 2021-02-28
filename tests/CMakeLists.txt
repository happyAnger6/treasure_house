aux_source_directory(memory TEST_SRCS)
aux_source_directory(container TEST_SRCS)

add_executable(test_all ${TEST_SRCS})

target_include_directories(test_all PUBLIC
                    ${PROJECT_ROOT}/src/include
                    ${PROJECT_ROOT}/src/container
                    ${PROJECT_ROOT}/src/memory)

target_link_libraries(test_all
                    PRIVATE treasure_house
                    PRIVATE gtest
                    PRIVATE gtest_main
                    PRIVATE pthread)
add_test(
    NAME gtest
    COMMAND $<TARGET_FILE:test_all>)
