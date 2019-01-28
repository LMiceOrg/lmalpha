cmake_minimum_required(VERSION 2.8)

project(factor_test)
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_BINARY_DIR}")

# os -- compiler pretarget setting
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    if (MSVC)
        #link_directories(${EXECUTABLE_OUTPUT_PATH} x64/Release)
		#target_link_directories(factor_test PUBLIC )
    endif()
endif()


LIST(APPEND factor_test_src factor_main.cpp)

ADD_LIBRARY(factor_test SHARED ${factor_test_src})

add_dependencies(factor_test lmapi lmlib)


# os -- compiler target setting
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
    target_link_libraries(factor_test lmapi lmlib)
    target_include_directories(factor_test PUBLIC ../include)

elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		target_link_directories(factor_test PUBLIC ${LIBRARY_OUTPUT_PATH})
        target_link_libraries(factor_test lmapi.lib lmlib.lib)
        target_include_directories(factor_test PUBLIC ../include)
        #target_link_libraries(factor_test ${Iconv_LIBRARIES} ${xml2_LIBRARIES} )
        #target_include_directories(factor_test PUBLIC ${Iconv_INCLUDE_DIRS} ${xml2_INCLUDE_DIRS} ${json_INCLUDE_DIRS})
    endif()
endif()

# posttarget setting
set(config_file "factor_cfg.json")

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
add_custom_command(TARGET factor_test  POST_BUILD
  COMMAND echo "Generating factor config file."
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${config_file}" "${CMAKE_BINARY_DIR}/${config_file}"
  COMMENT  "Preparing example/factor_test depends..."
 )

 endif()

