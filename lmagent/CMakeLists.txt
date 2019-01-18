cmake_minimum_required(VERSION 2.8)

project(lmagent)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        link_directories(${Iconv_LIBRARY_DIRS})
        link_directories(${xml2_LIBRARY_DIRS})
        link_directories(${json_LIBRARY_DIRS})
endif()

LIST(APPEND lmagent_src main.cpp

    )

add_executable(lmagent.exe ${lmagent_src})

target_link_libraries( lmagent.exe   eal_static lmapi)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    target_link_libraries(lmagent.exe ${Iconv_LIBRARIES} ${xml2_LIBRARIES} ${json_LIBRARIES})
    target_include_directories(lmagent.exe PUBLIC ${Iconv_INCLUDE_DIRS} ${xml2_INCLUDE_DIRS} ${json_INCLUDE_DIRS})

endif()
