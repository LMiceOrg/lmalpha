cmake_minimum_required(VERSION 3.11)

project(ALPHA)

message(STATUS "Current generate started  -G ${CMAKE_GENERATOR} ${CMAKE_SOURCE_DIR}")

#os -- compiler environment setting

message(STATUS "os: ${CMAKE_SYSTEM_NAME} compiler:${CMAKE_CXX_COMPILER_ID}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    #OpenCL
    find_package(OpenCL)
    if(OPENCL_FOUND)
        INCLUDE_DIRECTORIES(${OpenCL_INCLUDE_DIRS} /usr/local/include)
        LINK_DIRECTORIES(${OpenCL_LIBRARY})
    else()
        message(WARNING, "Not found OpenCL !")
    endif()

    #MPI
    find_package(MPI)
    if(MPI_FOUND)
        INCLUDE_DIRECTORIES(${MPI_INCLUDE_DIRS})
        LINK_DIRECTORIES(${MPI_LIBRARY})
    else()
        message(WARNING, "Not found MPI!")
    endif()

    #OpenMP
    find_package(OpenMP)
    if(OPENMP_FOUND)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    else()
        message(WARNING, "Not found OpenMP!")


    endif()

    if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU|Clang")
        add_definitions("-DUNICODE -D_UNICODE")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -Wall  -std=c++11 -march=native -mtune=native -m64")
        set(CMAKE_C_FLAGS   "${CMAKE_CXX_FLAGS} -g3 -Wall  -std=c90 -ansi -march=native -mtune=native -m64")
        message("darwin clang/gnu")
    endif()

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

    if (MSVC)
        add_definitions("-DUNICODE -D_UNICODE")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14")
        # set(CMAKE_C_FLAGS   "${CMAKE_CXX_FLAGS} /std:c90")

        SET(json_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/thirdpart/jansson/include")
        SET(json_LIBRARIES jansson)
        SET(json_LIBRARY_DIRS "${CMAKE_SOURCE_DIR}/thirdpart/jansson/x64/Release/")

    #    add_library(json STATIC IMPORTED) # or STATIC instead of SHARED
    #    set_property(TARGET json PROPERTY
    #        IMPORTED_LOCATION "d:/thirdpart/jansson/lib/Release/jansson.lib")
    #    set_property(TARGET json PROPERTY
    #      INTERFACE_INCLUDE_DIRECTORIES "d:/thirdpart/jansson/include"
    #    )

        SET(Iconv_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/thirdpart/iconv/include")
        SET(Iconv_LIBRARIES iconv)
        SET(Iconv_LIBRARY_DIRS "${CMAKE_SOURCE_DIR}/thirdpart/iconv/x64/Release")

    #    add_library(Iconv SHARED IMPORTED) # or STATIC instead of SHARED
    #    set_property(TARGET Iconv PROPERTY
    #        IMPORTED_LOCATION "d:/thirdpart/iconv/x64/Release/iconv.lib")
    #    set_property(TARGET Iconv PROPERTY
    #      INTERFACE_INCLUDE_DIRECTORIES "d:/thirdpart/iconv/include"
    #    )

        SET(xml2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/thirdpart/libxml2/include")
        SET(xml2_LIBRARIES libxml2)
        SET(xml2_LIBRARY_DIRS "${CMAKE_SOURCE_DIR}/thirdpart/libxml2/x64/Release")

    #    add_library(xml2 SHARED IMPORTED) # or STATIC instead of SHARED
    #    set_property(TARGET xml2 PROPERTY
    #        IMPORTED_LOCATION "d:/thirdpart/libxml2/libxml2.lib")
    #    set_property(TARGET xml2 PROPERTY
    #      INTERFACE_INCLUDE_DIRECTORIES "d:/thirdpart/libxml2"
    #    )

        SET(gsl_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/thirdpart/gsl/include")
        SET(gsl_LIBRARIES gsl cblas)
        SET(gsl_LIBRARY_DIRS "${CMAKE_SOURCE_DIR}/thirdpart/gsl/x64/Release")

    #    add_library(gsl STATIC IMPORTED) # or STATIC instead of SHARED
    #    set_property(TARGET gsl PROPERTY
    #        IMPORTED_LOCATION "d:/thirdpart/gsl/lib/x64/Release/gsl.lib d:/thirdpart/gsl/lib/x64/Release/cblas")
    #    set_property(TARGET gsl PROPERTY
    #      INTERFACE_INCLUDE_DIRECTORIES "d:/thirdpart/gsl"
    #    )

        message("MSVC: jansson ${json_INCLUDE_DIRS}")
    endif()

    if(MINGW)
        message("windows mingw")

        find_package(PkgConfig REQUIRED)

        add_definitions("-DUNICODE -D_UNICODE")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -mtune=native")
        set(CMAKE_C_FLAGS   "${CMAKE_CXX_FLAGS} -std=c90 -march=native -mtune=native")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -fexec-charset=gb18030")
        set(CMAKE_C_FLAGS "${CMAK_C_FLAGS}  -fexec-charset=gb18030")
    endif()


endif()



# EAL library
FIND_PATH(eal_header lmice_trace.h
    PATHS eal)
LIST(APPEND eal_SRCS
    #eal/lmice_ring.c
    eal/lmice_trace.c
    #eal/lmice_eal_shm.c
    #eal/lmice_eal_hash.c
    #eal/lmice_eal_spinlock.c
    #eal/lmice_eal_malloc.c
    #eal/lmice_eal_event.c
    eal/lmice_eal_time.c
    #eal/lmice_core.c
    #eal/lmice_eal_aio.c
    eal/lmice_eal_thread.c
    #eal/lmice_bloomfilter.c
    )

#Shared EAL Library
#ADD_LIBRARY(eal SHARED ${EAL_SRCS})
#Static EAL Library
ADD_LIBRARY(eal_static STATIC ${eal_SRCS})

SET_TARGET_PROPERTIES(eal_static PROPERTIES OUTPUT_NAME "eal")
SET_TARGET_PROPERTIES(eal_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#SET_TARGET_PROPERTIES(eal_static PROPERTIES VERSION 1.2 SOVERSION 1)


# Thunk library
LIST(APPEND thunklib_src thirdpart/call_thunk/call_thunk.cpp
    thirdpart/call_thunk/call_thunk.h)

ADD_LIBRARY(thunk_static STATIC ${thunklib_src})
SET_TARGET_PROPERTIES(thunk_static PROPERTIES OUTPUT_NAME "thunk")
SET_TARGET_PROPERTIES(thunk_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#SET_TARGET_PROPERTIES(thunk_static PROPERTIES VERSION 1.2 SOVERSION 1)

INCLUDE_DIRECTORIES(${eal_header})
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


message(STATUS "current generate done")

