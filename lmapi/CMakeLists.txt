cmake_minimum_required(VERSION 3.1)

project(lmapi)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")

pkg_search_module(unixodbc REQUIRED odbc)
pkg_search_module(json REQUIRED jansson)

LIST(APPEND lmapi_src lmapi.h
    lmapi.cpp
    lmsql.h
    lmsql.cpp
    lm_data_load.cpp
    lm_sql_query.cpp
    lm_config_read.cpp
    lm_console_log.cpp
    lm_result_store.cpp
)

ADD_LIBRARY(lmapi SHARED ${lmapi_src})
target_link_libraries(lmapi  ${json_LDFLAGS} ${unixodbc_LDFLAGS} eal_static)
target_include_directories(lmapi PUBLIC ${unixodbc_INCLUDE_DIRS})
target_compile_options(lmapi PUBLIC ${json_CFLAGS_OTHER} ${unixodbc_CFLAGS_OTHER})
