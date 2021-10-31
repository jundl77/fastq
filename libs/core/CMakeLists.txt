set(TARGET_NAME core_lib)

file(GLOB headers *.h)
file(GLOB sources *.cc)

set(sources
        ${headers} ${sources})

add_library(${TARGET_NAME} ${sources})

set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_options(${TARGET_NAME} PRIVATE ${cxx_compile_options})
target_include_directories(${TARGET_NAME} PUBLIC ..)