cmake_minimum_required(VERSION 2.8)

project(lmlib)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")

pkg_search_module(gsl REQUIRED gsl)

LIST(APPEND lmlib_src
    lm.cpp
    )

add_library(lmlib SHARED ${lmlib_src})
target_link_libraries(lmlib   ${gsl_LDFLAGS} )
target_include_directories(lmlib PUBLIC ${gsl_INCLUDE_DIRS} )
target_compile_options(lmlib PUBLIC ${gsl_CFLAGS_OTHER} )

