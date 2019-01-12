cmake_minimum_required(VERSION 2.8)

project(lmagent)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")

LIST(APPEND lmagent_src main.cpp

    )

add_executable(lmagent.exe ${lmagent_src})

target_link_libraries( lmagent.exe   eal_static lmapi)



