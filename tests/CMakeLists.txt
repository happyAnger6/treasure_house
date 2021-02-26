aux_source_directory(memory TEST_SRCS)
aux_source_directory(container TEST_FILES)

add_executable(test_all ${TEST_FILES})

target_include_directories(test_all PUBLIC
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/container)

target_link_libraries(test_all
                    PRIVATE gtest
                    PRIVATE gtest_main
                    PRIVATE pthread)
add_test(
    NAME gtest
    COMMAND $<TARGET_FILE:test_all>)
