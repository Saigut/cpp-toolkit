include_guard()

include(common_var)

function(compiler_set_compilation_options)
    # Compiler
    ## Firstly, chose generator, this is chosing build toochain. It's like only can chose generator on cmake command line.

    message("Number of logic cores of hardware: " ${LCORES})
    ## Which compiler
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
        # Using Clang
        set(CLANG true)
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        # Using GCC
        set(GCC true)
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Intel)
        # Using Intel C++
        set(INTEL true)
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
        # Using Visual Studio C++
        set(MSVC true)
    else()
        message(FATAL_ERROR "Unsupported compiler: " ${CMAKE_CXX_COMPILER_ID})
    endif()
    message("Using C Compiler: " ${CMAKE_C_COMPILER_ID})
    message("Using CXX Compiler: " ${CMAKE_CXX_COMPILER_ID})

    ## Which OS
    if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Linux)
        set(OS_LINUX true)
    elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
        set(OS_WINDOWS true)
    elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Darwin)
        set(OS_DARWIN true)
    else()
        message(FATAL_ERROR "Unsupported os: " ${CMAKE_HOST_SYSTEM_NAME})
    endif()
    message("OS: " ${CMAKE_HOST_SYSTEM_NAME})

    ## Settings for diferent OSes and compilers
    if(OS_LINUX AND GCC)
        add_definitions(-D_GNU_SOURCE)
    endif()

    ## Standard
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 11)

endfunction()
