set(TARGET_NAME fastq_tests)
include(gtest.cmake)

file(GLOB headers *.h)
file(GLOB sources *.cc)
add_executable(${TARGET_NAME} ${headers} ${sources})

find_package (Threads)
target_link_libraries(${TARGET_NAME}
        consumer_lib
        producer_lib
        gtest
        ${CMAKE_THREAD_LIBS_INIT})
add_test(${TARGET_NAME} unit_tests)
