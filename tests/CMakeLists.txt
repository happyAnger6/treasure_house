aux_source_directory(memory TEST_SRCS)
aux_source_directory(container TEST_SRCS)
aux_source_directory(threadpool TEST_SRCS)
aux_source_directory(atm TEST_SRCS)

add_executable(test_all ${TEST_SRCS})

target_include_directories(test_all PUBLIC
                    ${PROJECT_ROOT}/src/include
                    ${PROJECT_ROOT}/src/container
                    ${PROJECT_ROOT}/src/memory
                    ${PROJECT_ROOT}/src/threadpool
)

target_link_libraries(test_all
                    PUBLIC treasure_house
                    gtest
                    gtest_main
                    pthread
)

add_test(
    NAME gtest
    COMMAND $<TARGET_FILE:test_all>)

aux_source_directory(queue QUEUE_SRCS)
add_executable(test_queue ${QUEUE_SRCS})

target_include_directories(test_queue PUBLIC
                    ${PROJECT_ROOT}/src/include
                    ${PROJECT_ROOT}/src/container
                    ${PROJECT_ROOT}/src/queue
                    ${PROJECT_ROOT}/src/threadpool
)

target_link_libraries(test_queue
                    PUBLIC treasure_house
                    gtest
                    gtest_main
                    pthread
)

add_test(
    NAME gtest_queue
    COMMAND $<TARGET_FILE:test_queue>)

aux_source_directory(coroutine COROUTINE_SRCS)
add_executable(test_coroutine ${COROUTINE_SRCS})

target_include_directories(test_coroutine PUBLIC
                    ${PROJECT_ROOT}/src/include
                    ${PROJECT_ROOT}/src/container
                    ${PROJECT_ROOT}/src/coroutine
)

target_link_libraries(test_coroutine
                    PUBLIC treasure_house
                    gtest
                    gtest_main
                    pthread
)

add_test(
    NAME gtest_coroutine
    COMMAND $<TARGET_FILE:test_coroutine>)