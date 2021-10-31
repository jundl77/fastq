set(TARGET_NAME consumer_lib)

file(GLOB headers *.h)
file(GLOB sources *.cc)

set(sources
        ${headers} ${sources})

add_library(${TARGET_NAME} ${sources})
target_link_libraries(${TARGET_NAME} PUBLIC
        fastq_idl)

target_compile_options(${TARGET_NAME} PRIVATE ${cxx_compile_options})
target_include_directories(${TARGET_NAME} PUBLIC ..)