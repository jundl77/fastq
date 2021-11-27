set(TARGET_NAME test_producer_app)

file(GLOB headers *.h)
file(GLOB sources *.cc)

add_executable(${TARGET_NAME} ${headers} ${sources})
target_link_libraries(${TARGET_NAME}
        fastq_producer_lib
        fastq_core_lib)
target_include_directories(${TARGET_NAME} PUBLIC ..)