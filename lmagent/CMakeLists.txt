cmake_minimum_required(VERSION 3.11)

project(lmagent)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")

# os -- compiler pretarget setting
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")

    endif()

endif()

LIST(APPEND lmagent_src main.cpp)

add_executable(lmagent.exe ${lmagent_src})

target_link_libraries( lmagent.exe   eal_static lmapi )

#os -- compiler target setting
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        target_link_directories(lmagent.exe ${Iconv_LIBRARY_DIRS}
            ${xml2_LIBRARY_DIRS}
            json_LIBRARY_DIRS})
        target_link_libraries(lmagent.exe
            ${Iconv_LIBRARIES}
            ${xml2_LIBRARIES}
            ${json_LIBRARIES})
        target_include_directories(lmagent.exe PUBLIC
            ../${Iconv_INCLUDE_DIRS}
            ../${xml2_INCLUDE_DIRS}
            ../${json_INCLUDE_DIRS})

    endif()
endif()
