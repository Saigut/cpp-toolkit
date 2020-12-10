include_guard()

include(ExternalProject)
include(common_var)

function(external_lib_setup baseDir)
    # Parallel argument of build
    set(PARALLEL_ARG "")
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12")
        if(LCORES GREATER 0)
            math(EXPR LCORES_PLUS1 "${LCORES} + 1")
            set(PARALLEL_ARG "-j ${LCORES_PLUS1}")
        endif()
    endif()

    set(BOOST_BS_SUFFIX "")
    if(OS_WINDOWS)
        set(BOOST_BS_SUFFIX ".bat")
    else()
        set(BOOST_BS_SUFFIX ".sh")
    endif()

    # json-c
    ExternalProject_Add(ep_json-c
#        URL https://github.com/json-c/json-c/archive/master.zip
        URL ${baseDir}/package/json-c.zip
        DOWNLOAD_NAME json-c.zip
        CMAKE_ARGS
            -DBUILD_SHARED_LIBS=OFF
            -DCMAKE_INSTALL_PREFIX:PATH=${baseDir}/external
            -DBUILD_TESTING=OFF
        BUILD_COMMAND ${CMAKE_COMMAND} --build . ${PARALLEL_ARG})

    # boost_1_72_0
    ExternalProject_Add(ep_boost
#        URL https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.zip
        URL ${baseDir}/package/boost_1_72_0.zip
        CONFIGURE_COMMAND ./bootstrap${BOOST_BS_SUFFIX}
        BUILD_IN_SOURCE true
        BUILD_COMMAND ""
        INSTALL_COMMAND ./b2 --layout=system --prefix=${baseDir}/external ${PARALLEL_ARG} address-model=64 architecture=x86 variant=release link=static install)
endfunction()
