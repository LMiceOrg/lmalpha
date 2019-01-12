cmake_minimum_required(VERSION 2.8)

project(ALPHA)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    #shared lib suffix
    set(dll_ext ".dylib")

    #Set osx rpath
    set( CMAKE_MACOSX_RPATH 1)
    set( MACOSX_RPATH "@executable_path")
endif()


add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11> $<$<COMPILE_LANGUAGE:C>:-std=c90>)

find_package(PkgConfig REQUIRED)

FIND_PATH(eal_header lmice_trace.h
    PATHS ../fmtrader/eal)
LIST(APPEND eal_SRCS
    #../fmtrader/eal/lmice_ring.c
    ../fmtrader/eal/lmice_trace.c
    ../fmtrader/eal/lmice_eal_shm.c
    ../fmtrader/eal/lmice_eal_hash.c
    ../fmtrader/eal/lmice_eal_spinlock.c
    ../fmtrader/eal/lmice_eal_malloc.c
    ../fmtrader/eal/lmice_eal_event.c
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


add_subdirectory(lmagent)
add_subdirectory(lmlib)
add_subdirectory(lmapi)
add_subdirectory(doc)
add_subdirectory(example/factor_test)

