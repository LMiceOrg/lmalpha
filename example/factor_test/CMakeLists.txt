cmake_minimum_required(VERSION 2.8)

project(factor_test)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")

LIST(APPEND factor_test_src factor_main.cpp)

ADD_LIBRARY(factor_test SHARED ${factor_test_src})

target_link_libraries(factor_test lmapi)
target_include_directories(factor_test PUBLIC ../../lmapi)
