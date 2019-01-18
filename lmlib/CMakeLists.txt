cmake_minimum_required(VERSION 2.8)

project(lmlib)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
#SET(ENV{PKG_CONFIG_PATH} $ENV{PKG_CONFIG_PATH}

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
else()
pkg_search_module(gsl REQUIRED gsl)
endif()

message("gsl: ${gsl_INCLUDE_DIRS}")
LIST(APPEND lmlib_src
    lm.cpp
    )
link_directories(${gsl_LIBRARY_DIRS})
add_library(lmlib SHARED ${lmlib_src})




if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    target_link_libraries(lmlib   gsl cblas )
    target_include_directories(lmlib PUBLIC ${gsl_INCLUDE_DIRS} )
    target_compile_options(lmlib PUBLIC ${gsl_CFLAGS_OTHER} )

else()
target_link_libraries(lmlib  ${gsl_LDFLAGS} )
target_include_directories(lmlib PUBLIC ${gsl_INCLUDE_DIRS} )
target_compile_options(lmlib PUBLIC ${gsl_CFLAGS_OTHER} )
endif()
 
