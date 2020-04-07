include_guard()

include(ExternalProject)
include(common_var)

function(external_lib_setup)
    # Parallel argument of build
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12")
        set(PARALLEL_ARG "-j ${LCORES_PLUS1}")
    elseif()
        set(PARALLEL_ARG "")
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
endfunction()
