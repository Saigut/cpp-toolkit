include_guard()

include(ExternalProject)
include(common_var)

function(external_lib_setup)
    # json-c
    ExternalProject_Add(ep_json-c
        URL https://github.com/json-c/json-c/archive/master.zip
#        URL ${PROJECT_SOURCE_DIR}/package/json-c.zip
        DOWNLOAD_NAME json-c.zip
        CMAKE_ARGS
            -DBUILD_SHARED_LIBS=OFF
            -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_SOURCE_DIR}/external
            -DBUILD_TESTING=OFF
        BUILD_COMMAND ${CMAKE_COMMAND} --build . -j ${LCORES_PLUS1})
endfunction()
