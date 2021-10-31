set(TARGET_NAME producer_lib)

file(GLOB headers *.h)
file(GLOB sources *.cc)

add_library(${TARGET_NAME} ${headers} ${sources})
target_link_libraries(${TARGET_NAME} PUBLIC
        fastq_idl
        core_lib)

target_compile_options(${TARGET_NAME} PRIVATE ${cxx_compile_options})
target_include_directories(${TARGET_NAME} PUBLIC ..)
