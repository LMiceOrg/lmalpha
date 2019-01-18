cmake_minimum_required(VERSION 2.8)

project(ALPHA)

if(MINGW)
    message("mingw ${PKG_CONFIG_PATH}")
    add_definitions("-DUNICODE -D_UNICODE")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -fexec-charset=utf-8")
    #set(CMAKE_C_FLAGS "${CMAK_C_FLAGS}  -fexec-charset=utf-8")
endif()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    SET(json_INCLUDE_DIRS d:/thirdpart/jansson/include)
    SET(json_LIBRARIES jansson)
    SET(json_LIBRARY_DIRS d:/thirdpart/jansson/lib/Release/)

#    add_library(json STATIC IMPORTED) # or STATIC instead of SHARED
#    set_property(TARGET json PROPERTY
#        IMPORTED_LOCATION "d:/thirdpart/jansson/lib/Release/jansson.lib")
#    set_property(TARGET json PROPERTY
#      INTERFACE_INCLUDE_DIRECTORIES "d:/thirdpart/jansson/include"
#    )

    SET(Iconv_INCLUDE_DIRS d:/thirdpart/iconv/include)
    SET(Iconv_LIBRARIES iconv)
    SET(Iconv_LIBRARY_DIRS d:/thirdpart/iconv/x64/Release)

#    add_library(Iconv SHARED IMPORTED) # or STATIC instead of SHARED
#    set_property(TARGET Iconv PROPERTY
#        IMPORTED_LOCATION "d:/thirdpart/iconv/x64/Release/iconv.lib")
#    set_property(TARGET Iconv PROPERTY
#      INTERFACE_INCLUDE_DIRECTORIES "d:/thirdpart/iconv/include"
#    )

    SET(xml2_INCLUDE_DIRS d:/thirdpart/libxml2)
    SET(xml2_LIBRARIES libxml2)
    SET(xml2_LIBRARY_DIRS d:/thirdpart/libxml2)

#    add_library(xml2 SHARED IMPORTED) # or STATIC instead of SHARED
#    set_property(TARGET xml2 PROPERTY
#        IMPORTED_LOCATION "d:/thirdpart/libxml2/libxml2.lib")
#    set_property(TARGET xml2 PROPERTY
#      INTERFACE_INCLUDE_DIRECTORIES "d:/thirdpart/libxml2"
#    )

    SET(gsl_INCLUDE_DIRS d:/thirdpart/gsl)
    SET(gsl_LIBRARIES gsl cblas)
    SET(gsl_LIBRARY_DIRS d:/thirdpart/gsl/lib/x64/Release)

#    add_library(gsl STATIC IMPORTED) # or STATIC instead of SHARED
#    set_property(TARGET gsl PROPERTY
#        IMPORTED_LOCATION "d:/thirdpart/gsl/lib/x64/Release/gsl.lib d:/thirdpart/gsl/lib/x64/Release/cblas")
#    set_property(TARGET gsl PROPERTY
#      INTERFACE_INCLUDE_DIRECTORIES "d:/thirdpart/gsl"
#    )

    message("MSVC: jansson ${json_INCLUDE_DIRS}")
endif()

message("compiler: ${CMAKE_CXX_COMPILER_ID}")

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU|Clang")
    add_definitions("-DUNICODE -D_UNICODE")
    set(CMAKE_CXX_FLAGS "-Wall  -std=c++11 -march=native -mtune=native")
    set(CMAKE_C_FLAGS   "-Wall  -std=c90 -march=native -mtune=native")

elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    add_definitions("-DUNICODE -D_UNICODE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -mtune=native")
    set(CMAKE_C_FLAGS   "${CMAKE_CXX_FLAGS} -std=c90 -march=native -mtune=native")
endif()

message("os: ${CMAKE_SYSTEM_NAME}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    find_package(PkgConfig REQUIRED)

    #shared lib suffix
    set(dll_ext ".dylib")

    #Set osx rpath
    set( CMAKE_MACOSX_RPATH 1)
    set( MACOSX_RPATH "@executable_path")
    #add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11> $<$<COMPILE_LANGUAGE:C>:-std=c90>)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -fexec-charset=utf-8")
    set(CMAKE_C_FLAGS "${CMAK_C_FLAGS}  -fexec-charset=utf-8")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -fexec-charset=gb18030")
 set(CMAKE_C_FLAGS "${CMAK_C_FLAGS}  -fexec-charset=gb18030")
endif()

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)


FIND_PATH(eal_header lmice_trace.h
    PATHS ../fmtrader/eal)
LIST(APPEND eal_SRCS
    #../fmtrader/eal/lmice_ring.c
    ../fmtrader/eal/lmice_trace.c
    #../fmtrader/eal/lmice_eal_shm.c
    #../fmtrader/eal/lmice_eal_hash.c
    #../fmtrader/eal/lmice_eal_spinlock.c
    #../fmtrader/eal/lmice_eal_malloc.c
    #../fmtrader/eal/lmice_eal_event.c
    ../fmtrader/eal/lmice_eal_time.c
    #../fmtrader/eal/lmice_core.c
    #../fmtrader/eal/lmice_eal_aio.c
    ../fmtrader/eal/lmice_eal_thread.c
    #../fmtrader/eal/lmice_bloomfilter.c
    )

#Shared EAL Library
#ADD_LIBRARY(eal SHARED ${EAL_SRCS})
#Static EAL Library
ADD_LIBRARY(eal_static STATIC ${eal_SRCS})

SET_TARGET_PROPERTIES(eal_static PROPERTIES OUTPUT_NAME "eal")
SET_TARGET_PROPERTIES(eal_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#SET_TARGET_PROPERTIES(eal_static PROPERTIES VERSION 1.2 SOVERSION 1)

INCLUDE_DIRECTORIES(${eal_header})

# Thunk library
LIST(APPEND thunklib_src thirdpart/call_thunk/call_thunk.cpp
    thirdpart/call_thunk/call_thunk.h)

ADD_LIBRARY(thunk_static STATIC ${thunklib_src})
SET_TARGET_PROPERTIES(thunk_static PROPERTIES OUTPUT_NAME "thunk")
SET_TARGET_PROPERTIES(thunk_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#SET_TARGET_PROPERTIES(thunk_static PROPERTIES VERSION 1.2 SOVERSION 1)

INCLUDE_DIRECTORIES(thirdpart/call_thunk)

INCLUDE_DIRECTORIES(include)

LIST(APPEND cfg_files example/lmalpha.xml
    example/lmalpha.json
    example/factor_test/factor_cfg.json
)
add_custom_target(example ALL SOURCES  ${cfg_files})



add_subdirectory(lmagent)
add_subdirectory(lmlib)
add_subdirectory(lmapi)
add_subdirectory(doc)
add_subdirectory(example/factor_test)

