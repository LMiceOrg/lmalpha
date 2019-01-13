cmake_minimum_required(VERSION 3.1)

project(lmapi)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")

pkg_search_module(unixodbc REQUIRED odbc)
pkg_search_module(json REQUIRED jansson)
pkg_search_module(xml2 REQUIRED libxml-2.0 libxml2 libxml>=2)

add_library( json_static_lib STATIC IMPORTED )
SET_TARGET_PROPERTIES(json_static_lib PROPERTIES IMPORTED_LOCATION
    "${json_STATIC_LIBRARY_DIRS}/libjansson.a")

add_library( unixodbc_static_lib STATIC IMPORTED )
SET_TARGET_PROPERTIES(unixodbc_static_lib PROPERTIES IMPORTED_LOCATION
    "${unixodbc_STATIC_LIBRARY_DIRS}/libodbc.a")

message("xml2:${xml2_LDFLAGS}")
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
# shared link
#target_link_libraries(lmapi  json_static_lib ${unixodbc_LDFLAGS} eal_static)
# static link
target_link_libraries(lmapi  json_static_lib ${unixodbc_LDFLAGS} eal_static ${xml2_LDFLAGS})
target_include_directories(lmapi PUBLIC ${unixodbc_INCLUDE_DIRS} ${xml2_INCLUDE_DIRS})
target_compile_options(lmapi PUBLIC ${json_CFLAGS_OTHER} ${unixodbc_CFLAGS_OTHER})


LIST( APPEND config_file "lmalpha.json"
    "lmalpha.xml")
#add_custom_command(TARGET lmapi  POST_BUILD
#  COMMAND echo "Generating alpha framework config file."
#  COMMAND ${CMAKE_COMMAND} -E copy ${config_file} "${CMAKE_BINARY_DIR}/${config_file}"
#  COMMENT  "Preparing example/lmalpha depends..."
# )

add_custom_command(
    TARGET lmapi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E tar cfj ${CMAKE_BINARY_DIR}/temp.tar ${config_file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../example
    COMMENT  "Preparing example/lmalpha depends..."
)

add_custom_command(
    TARGET lmapi POST_BUILD
    #COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_BINARY_DIR}/temp.tar temp.tar
    COMMAND ${CMAKE_COMMAND} -E tar xfj temp.tar ${FileList}
    COMMAND ${CMAKE_COMMAND} -E remove temp.tar
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}

)
