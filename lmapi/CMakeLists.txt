cmake_minimum_required(VERSION 2.8)

project(lmapi)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")

FIND_LIBRARY(Iconv_LIBRARIES NAMES iconv libiconv)
FIND_PATH(Iconv_INCLUDE_DIRS iconv.h)

message("iconv: ${Iconv_INCLUDE_DIRS}")
pkg_search_module(json REQUIRED jansson)
pkg_search_module(xml2 REQUIRED libxml-2.0 libxml2 libxml>=2)

add_library( json_static_lib STATIC IMPORTED )
SET_TARGET_PROPERTIES(json_static_lib PROPERTIES IMPORTED_LOCATION
    "${json_STATIC_LIBRARY_DIRS}/libjansson.a")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
pkg_search_module(unixodbc REQUIRED odbc)
add_library( unixodbc_static_lib STATIC IMPORTED )
SET_TARGET_PROPERTIES(unixodbc_static_lib PROPERTIES IMPORTED_LOCATION
    "${unixodbc_STATIC_LIBRARY_DIRS}/libodbc.a")
endif()

message("xml2:${xml2_INCLUDE_DIRS}")
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
INCLUDE_DIRECTORIES( "${xml2_INCLUDE_DIRS}" "${json_INCLUDE_DIRS}")
target_link_libraries(lmapi  ${json_LDFLAGS} ${unixodbc_LDFLAGS} eal_static ${xml2_LDFLAGS})
target_include_directories(lmapi PUBLIC ${unixodbc_INCLUDE_DIRS} ${xml2_INCLUDE_DIRS} ${json_INCLUDE_DIRS})
target_compile_options(lmapi PUBLIC ${json_CFLAGS_OTHER} ${unixodbc_CFLAGS_OTHER})

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

target_link_libraries(lmapi odbc32)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(lmapi ${Iconv_LIBRARIES})
    target_include_directories(lmapi PUBLIC ${Iconv_INCLUDE_DIRS})
    target_compile_options(lmapi PUBLIC ${Iconv_CFLAGS_OTHER})
endif()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_link_libraries(lmapi ${Iconv_LIBRARIES})
    target_include_directories(lmapi PUBLIC ${Iconv_INCLUDE_DIRS})
    target_compile_options(lmapi PUBLIC ${Iconv_CFLAGS_OTHER})
endif()

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
