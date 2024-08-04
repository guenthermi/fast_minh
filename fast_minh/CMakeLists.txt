cmake_minimum_required(VERSION 3.10)

project(FastMinh VERSION 0.1.0)

add_library(fast_minh SHARED fast_minh.cpp MurmurHash3.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tests/cpp ${CMAKE_CURRENT_BINARY_DIR}/tests)
endif()
