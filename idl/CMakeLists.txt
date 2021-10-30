set(TARGET_NAME fastq_idl)

SET(FLATBUFFERS_FLATC_SCHEMA_EXTRA_ARGS "--gen-all")

file(GLOB fbs "*.fbs")

# generate cpp files from idl files
set(generated_files "")
foreach(fb ${fbs})
    get_filename_component(stem ${fb} NAME_WE)
    get_filename_component(name ${fb} NAME)
    set(generated_include ${CMAKE_CURRENT_BINARY_DIR}/${stem}_generated.h)
    add_custom_command(
            OUTPUT ${generated_include}
            COMMENT "Compiling flatbuffer for ${name}"
            COMMAND flatc
            --cpp
            --scoped-enums
            --reflect-types
            --reflect-names
            -o ${CMAKE_CURRENT_BINARY_DIR}
            -I ${CMAKE_CURRENT_SOURCE_DIR}
            ${fb}
            DEPENDS flatc ${fb}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    list(APPEND generated_files ${generated_include})
endforeach()

add_library(${TARGET_NAME}
        ${generated_files})
target_link_libraries(${TARGET_NAME} PUBLIC flatbuffers)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
