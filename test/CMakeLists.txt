aux_source_directory(. TEST_FILES)
aux_source_directory(./container TEST_FILES)

add_executable(test_all ${TEST_FILES})

target_link_libraries(test_all PUBLIC
                    gtest
                    pthread)