set(TARGET_NAME test_consumer_app)

file(GLOB headers *.h)
file(GLOB sources *.cc)

add_executable(${TARGET_NAME} ${headers} ${sources})
target_link_libraries(${TARGET_NAME}
        consumer_lib
        core_lib)
