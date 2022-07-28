cmake_minimum_required(VERSION 3.5.1)

project(fastq LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("Building for Mac OSX")
    set(MACOSX TRUE)
else()
    message("Not building for Mac OSX")
    set(MACOSX FALSE)
endif()

add_subdirectory(libs)
add_subdirectory(unit_tests)
add_subdirectory(sample_apps)
