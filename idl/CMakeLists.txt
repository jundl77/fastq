set(TARGET_NAME fastq_idl)

add_library(${TARGET_NAME} fastq.h)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES} stdc++fs)
else()
    target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
endif()

set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_options(${TARGET_NAME} PRIVATE ${cxx_compile_options})
target_include_directories(${TARGET_NAME} PUBLIC ..)
