include_guard()


function(compiler_set_compilation_options)
    # Compiler
    ## GCC
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)

    ## Standard
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 11)

    # Settings for diferent OSes and compilers
    if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Linux)
        if(${CMAKE_C_COMPILER} STREQUAL gcc)
#            add_definitions(-D_GNU_SOURCE)
        endif()
    elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Darwin )
    else()
        message(FATAL_ERROR "Unsupported os: " ${CMAKE_HOST_SYSTEM_NAME})
    endif()

#    if(${CMAKE_C_COMPILER} STREQUAL gcc)
#    elseif(${CMAKE_C_COMPILER} STREQUAL msvc)
#    endif()

endfunction()

