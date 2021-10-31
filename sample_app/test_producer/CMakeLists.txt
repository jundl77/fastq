set(TARGET_NAME test_producer_app)

file(GLOB headers *.h)
file(GLOB sources *.cc)

add_executable(${TARGET_NAME} ${headers} ${sources})
target_link_libraries(${TARGET_NAME}
        producer_lib
        core_lib)
