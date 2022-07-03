include_guard()

include(common_var)

macro(compiler_set_compilation_options)
    # Compiler
    ## Firstly, chose generator, this is chosing build toochain. It's like only can chose generator on cmake command line.

    message("Number of logic cores of hardware: " ${GV_LCORES})
    ## Which compiler
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
        # Using Clang
        set(CLANG true)
        add_definitions(-D_CPPT_CPL_CLANG)
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        # Using GCC
        set(GCC true)
        add_definitions(-D_CPPT_CPL_GCC)
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Intel)
        # Using Intel C++
        set(INTEL true)
        add_definitions(-D_CPPT_CPL_INTEL)
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
        # Using Visual Studio C++
        set(MSVC true)
        add_definitions(-D_CPPT_CPL_MSVC)
    else()
        message(FATAL_ERROR "Unsupported compiler: " ${CMAKE_CXX_COMPILER_ID})
    endif()
    message("Using C Compiler: " ${CMAKE_C_COMPILER_ID})
    message("Using CXX Compiler: " ${CMAKE_CXX_COMPILER_ID})

    ## Which OS
    if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Linux)
        set(OS_LINUX true)
        add_definitions(-D_CPPT_OS_LINUX)
    elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
        set(OS_WINDOWS true)
        add_definitions(-D_CPPT_OS_WINDOWS)
    elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Darwin)
        set(OS_DARWIN true)
        add_definitions(-D_CPPT_OS_DARWIN)
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
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(-DJSON_C_HAVE_INTTYPES_H=1) # for json-c
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
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
