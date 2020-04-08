include_guard()

include(ExternalProject)
include(common_var)

function(external_lib_setup)
    # Parallel argument of build
    set(PARALLEL_ARG "")
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12")
        if(LCORES GREATER 0)
            math(EXPR LCORES_PLUS1 "${LCORES} + 1")
            set(PARALLEL_ARG "-j ${LCORES_PLUS1}")
        endif()
    endif()

    # json-c
    ExternalProject_Add(ep_json-c
        URL https://github.com/json-c/json-c/archive/master.zip
#        URL ${PROJECT_SOURCE_DIR}/package/json-c.zip
        DOWNLOAD_NAME json-c.zip
        CMAKE_ARGS
            -DBUILD_SHARED_LIBS=OFF
            -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_SOURCE_DIR}/external
            -DBUILD_TESTING=OFF
        BUILD_COMMAND ${CMAKE_COMMAND} --build . ${PARALLEL_ARG})

    # boost_1_72_0
    ExternalProject_Add(ep_boost
        URL ${PROJECT_SOURCE_DIR}/package/boost_1_72_0.zip
        CONFIGURE_COMMAND ./bootstrap
        BUILD_IN_SOURCE true
        BUILD_COMMAND ./b2 ${PARALLEL_ARG}
        INSTALL_COMMAND ./b2 --prefix=${PROJECT_SOURCE_DIR}/external link=static install)
endfunction()
