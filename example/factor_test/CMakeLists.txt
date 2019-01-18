cmake_minimum_required(VERSION 2.8)

project(factor_test)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        link_directories(${Iconv_LIBRARY_DIRS})
        link_directories(${xml2_LIBRARY_DIRS})
        link_directories(${json_LIBRARY_DIRS})
        link_directories(${gsl_LIBRARY_DIRS})
endif()


LIST(APPEND factor_test_src factor_main.cpp)

ADD_LIBRARY(factor_test SHARED ${factor_test_src})

target_link_libraries(factor_test lmapi lmlib)
target_include_directories(factor_test PUBLIC ../../lmapi ../include)

set(config_file "factor_cfg.json")
add_custom_command(TARGET factor_test  POST_BUILD
  COMMAND echo "Generating factor config file."
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${config_file}" "${CMAKE_BINARY_DIR}/${config_file}"
  COMMENT  "Preparing example/factor_test depends..."
 )


if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    target_link_libraries(factor_test ${Iconv_LIBRARIES} ${xml2_LIBRARIES} )
    target_include_directories(factor_test PUBLIC ${Iconv_INCLUDE_DIRS} ${xml2_INCLUDE_DIRS} ${json_INCLUDE_DIRS})
endif()
