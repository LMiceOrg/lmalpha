option(LMALPHA_RECURSIVE_GENERATE "Recursive call to cmake" OFF)

macro(win_path src dst)
    string(REPLACE "/"  "\\" tmp_str ${src})
    set(${dst} ${tmp_str})
endmacro()
macro(win_project_rpath_refine subdir vcproj)
    set(vcproj_name "${CMAKE_BINARY_DIR}/${subdir}/${vcproj}.vcxproj")

    set(replace_string "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/\$\\(SolutionDir\\)")
    string(REPLACE "\$"  "\\\$" replace_string ${replace_string})

    set(to_string "\$(SolutionDir)")

    message("(gc ${vcproj_name}) -replace '${replace_string}', '${to_string}' | Out-File ${vcproj_name}.txt")
    execute_process(COMMAND
        powershell
        -Command
        """(gc ${vcproj_name}) -replace '${replace_string}', '${to_string}' | Out-File -encoding UTF8 ${vcproj_name}"""
    )

    win_path("${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/\$(SolutionDir)" replace_string)
    string(REPLACE "\\"  "\\\\" replace_string ${replace_string})
    string(REPLACE "\$"  "\\\$" replace_string ${replace_string})
    string(REPLACE "("  "\\(" replace_string ${replace_string})
    string(REPLACE ")"  "\\)" replace_string ${replace_string})

    message("(gc ${vcproj_name}) -replace '${replace_string}', '${to_string}' | Out-File ${vcproj_name}.txt")
    execute_process(COMMAND
        powershell
        -Command
        """(gc ${vcproj_name}) -replace '${replace_string}', '${to_string}' | Out-File -encoding UTF8 ${vcproj_name}"""
    )
endmacro()


macro(recursize_call)
    if(NOT LMALPHA_RECURSIVE_GENERATE)
    #    message(STATUS "Recursive generate started  -G ${CMAKE_GENERATOR} ${CMAKE_SOURCE_DIR}")
    #    execute_process(COMMAND ${CMAKE_COMMAND}
    #        -G "${CMAKE_GENERATOR}"
    #        -T "${CMAKE_GENERATOR_TOOLSET}"
    #        -A "${CMAKE_GENERATOR_PLATFORM}"
    #        -DLMALPHA_RECURSIVE_GENERATE:BOOL=ON
    #        ${CMAKE_SOURCE_DIR})
    #    message(STATUS "Recursive generate done")

        # your post-generate steps here
        message("post generate started")
        if (MSVC)
            #win_project_rpath_refine(lmlib lmlib)
            #win_project_rpath_refine(lmapi lmapi)
            #win_project_rpath_refine(lmagent lmagent.exe)
            #win_project_rpath_refine(example/factor_test factor_test)
    #        set(vcproj_name "${CMAKE_BINARY_DIR}/lmlib/lmlib.vcxproj")

    #        set(replace_string "${CMAKE_CURRENT_SOURCE_DIR}/lmlib/\$\\(SolutionDir\\)")
    #        string(REPLACE "\$"  "\\\$" replace_string ${replace_string})

    #        set(to_string "\$(SolutionDir)")

    #        message("(gc ${vcproj_name}) -replace '${replace_string}', '${to_string}' | Out-File ${vcproj_name}.txt")
    #        execute_process(COMMAND
    #            powershell
    #            -Command
    #            """(gc ${vcproj_name}) -replace '${replace_string}', '${to_string}' | Out-File -encoding UTF8 ${vcproj_name}"""
    #        )

    #        win_path("${CMAKE_CURRENT_SOURCE_DIR}/lmlib/\$(SolutionDir)" replace_string)
    #        string(REPLACE "\\"  "\\\\" replace_string ${replace_string})
    #        string(REPLACE "\$"  "\\\$" replace_string ${replace_string})
    #        string(REPLACE "("  "\\(" replace_string ${replace_string})
    #        string(REPLACE ")"  "\\)" replace_string ${replace_string})

    #        message("(gc ${vcproj_name}) -replace '${replace_string}', '${to_string}' | Out-File ${vcproj_name}.txt")
    #        execute_process(COMMAND
    #            powershell
    #            -Command
    #            """(gc ${vcproj_name}) -replace '${replace_string}', '${to_string}' | Out-File -encoding UTF8 ${vcproj_name}"""
    #        )
            #MESSAGE(FATAL_ERROR  "post generate done")
        endif()
        # exit without doing anything else, since it already happened
        #return()
    endif()
endmacro()
