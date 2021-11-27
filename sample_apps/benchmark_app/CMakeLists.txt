set(TARGET_NAME benchmark_app)

file(GLOB headers *.h)
file(GLOB sources *.cc)

add_executable(${TARGET_NAME} ${headers} ${sources})
target_link_libraries(${TARGET_NAME}
        core_lib)

target_include_directories(${TARGET_NAME} PUBLIC ..)