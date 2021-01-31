include_guard()

include(common_var)

macro(compiler_set_compilation_options)
    # Compiler
    set(CMAKE_CXX_STANDARD 11)
    ## Firstly, chose generator, this is chosing build toochain. It's like only can chose generator on cmake command line.

    message("Number of logic cores of hardware: " ${GV_LCORES})
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
    if (OS_LINUX)
        set(PLATFORM_LINK_LIB pthread dl m)
    endif ()
    if(OS_LINUX AND GCC)
        add_definitions(-D_GNU_SOURCE)
    endif()
    if (MSVC)
        add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
        add_definitions(-D_WIN32_WINNT=0x0601)
    endif ()
    if (OS_WINDOWS)
        # _WIN32_WINNT for boost
        add_definitions(-D_WIN32_WINNT=0x0601)
    endif()
    if (OS_WINDOWS AND GCC)
        set(PLATFORM_LINK_LIB ${PLATFORM_LINK_LIB} ws2_32)
    endif()
    if (OS_WINDOWS AND CLANG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
    endif()
    add_definitions(-DBOOST_ALL_NO_LIB)

    ## Standard
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 11)

endmacro()
