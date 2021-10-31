cmake_minimum_required(VERSION 3.5.1)

project(fastq LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "Build unit tests (dep on gtest)")

find_package(Boost 1.73 REQUIRED system)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_subdirectory(idl)
add_subdirectory(libs)
add_subdirectory(unit_tests)
add_subdirectory(sample_app)
