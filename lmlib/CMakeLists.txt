cmake_minimum_required(VERSION 3.11)

project(lmlib)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")

message("lmlib ")
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    pkg_search_module(gsl REQUIRED gsl)
    link_directories(${gsl_LIBRARY_DIRS})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    if (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")

    endif()

endif()

message("gsl: ${gsl_INCLUDE_DIRS}")

LIST(APPEND lmlib_src
    lm.cpp
    )

add_library(lmlib SHARED ${lmlib_src})

message("echo ${PROJECT_BINARY_DIR}")

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
    target_link_libraries(lmlib  ${gsl_LDFLAGS} )
    target_include_directories(lmlib PUBLIC ${gsl_INCLUDE_DIRS} )
    target_compile_options(lmlib PUBLIC ${gsl_CFLAGS_OTHER} )

elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
        target_link_directories(lmlib PUBLIC  "${gsl_LIBRARY_DIRS}")
        target_link_libraries(lmlib   gsl cblas )
        target_include_directories(lmlib PUBLIC "${gsl_INCLUDE_DIRS}" )
        #set_property(TARGET lmlib PROPERTY
        #    INTERFACE_INCLUDE_DIRECTORIES "${gsl_INCLUDE_DIRS}")


    else()
        target_link_libraries(lmlib  ${gsl_LDFLAGS} )
        target_include_directories(lmlib PUBLIC ${gsl_INCLUDE_DIRS} )
        target_compile_options(lmlib PUBLIC ${gsl_CFLAGS_OTHER} )
    endif()
endif()

