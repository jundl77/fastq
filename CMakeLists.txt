cmake_minimum_required(VERSION 3.5.1)

project(fastq LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "Build unit tests (dep on gtest)")

# add flatbuffers
set(FLATBUFFERS_SRC_DIR ${CMAKE_SOURCE_DIR}/x_flatbuffers)
add_subdirectory(${FLATBUFFERS_SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/flatbuffers-build EXCLUDE_FROM_ALL)

add_subdirectory(idl)
add_subdirectory(libs)
add_subdirectory(unit_tests)
add_subdirectory(sample_app)
