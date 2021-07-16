include_guard()

include(ExternalProject)
include(common_var)

function(external_lib_setup baseDir)
    # Parallel argument of build
    set(PARALLEL_ARG "")
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12")
        if(GV_LCORES GREATER 0)
            set(PARALLEL_ARG "-j ${GV_LCORES_PLUS1}")
        endif()
    endif()

    set(BOOST_BS_SUFFIX "")
    if(OS_WINDOWS)
        set(BOOST_BS_SUFFIX ".bat")
    else()
        set(BOOST_BS_SUFFIX ".sh")
    endif()

    # setup package URL
    set(V_JSON_C_URL ${baseDir}/package/json-c.zip)
    set(V_BOOST_URL ${baseDir}/package/boost_1_72_0.zip)
    set(V_GRPC_URL ${baseDir}/package/grpc-1.34.0_with_deps.zip)
    if(NOT EXISTS "${V_JSON_C_URL}")
        set(V_JSON_C_URL https://github.com/json-c/json-c/archive/master.zip)
    endif()
    if(NOT EXISTS "${V_BOOST_URL}")
        set(V_BOOST_URL https://boostorg.jfrog.io/artifactory/main/release/1.72.0/source/boost_1_72_0.zip)
    endif()
    if(NOT EXISTS "${V_GRPC_URL}")
        set(V_GRPC_URL https://github.com/Saigut/grpc/releases/download/v1.34.0/grpc-1.34.0_with_deps.zip)
    endif()

    # json-c
    ExternalProject_Add(ep_json-c
        URL ${V_JSON_C_URL}
        DOWNLOAD_NAME json-c.zip
        CMAKE_ARGS
            -DBUILD_SHARED_LIBS=OFF
            -DCMAKE_INSTALL_PREFIX:PATH=${baseDir}/external/${CMAKE_CXX_COMPILER_ID}
            -DBUILD_TESTING=OFF
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        BUILD_COMMAND ${CMAKE_COMMAND} --build . ${PARALLEL_ARG})

    # boost_1_72_0
    ExternalProject_Add(ep_boost
        URL ${V_BOOST_URL}
        CONFIGURE_COMMAND ./bootstrap${BOOST_BS_SUFFIX}
        BUILD_IN_SOURCE true
        BUILD_COMMAND ""
        INSTALL_COMMAND ./b2 --layout=system --prefix=${baseDir}/external/${CMAKE_CXX_COMPILER_ID} ${PARALLEL_ARG} address-model=64 architecture=x86 variant=release link=static install)

    # grpc-1.34.0
    ExternalProject_Add(ep_grpc
#        git clone -b v1.34.0 https://github.com/grpc/grpc
        URL ${V_GRPC_URL}
        DOWNLOAD_NAME grpc-1.34.0_with_deps.zip
#        UPDATE_COMMAND git submodule update --init
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=${baseDir}/external/${CMAKE_CXX_COMPILER_ID}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        BUILD_COMMAND ${CMAKE_COMMAND} --build . ${PARALLEL_ARG})

    # gsl
    set(V_GSL_URL ${baseDir}/package/gsl-3.1.0.zip)
    if(NOT EXISTS "${V_GSL_URL}")
        set(V_GSL_URL https://github.com/microsoft/GSL/archive/refs/tags/v3.1.0.zip)
    endif()
    ExternalProject_Add(ep_gsl
        URL ${V_GSL_URL}
        DOWNLOAD_NAME gsl-3.1.0.zip
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${baseDir}/external/${CMAKE_CXX_COMPILER_ID}
        -DGSL_TEST=OFF
        -DGSL_INSTALL=ON
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        BUILD_COMMAND ${CMAKE_COMMAND} --build . ${PARALLEL_ARG})

#    # soralog
#    set(V_SORALOG_URL ${baseDir}/package/soralog-0.0.9.zip)
#    if(NOT EXISTS "${V_SORALOG_URL}")
#        set(V_SORALOG_URL https://github.com/xDimon/soralog/archive/refs/tags/v0.0.9.zip)
#    endif()
#    ExternalProject_Add(ep_soralog
#        URL ${V_SORALOG_URL}
#        DOWNLOAD_NAME soralog-0.0.9.zip
#        CMAKE_ARGS
#        -DCMAKE_INSTALL_PREFIX:PATH=${baseDir}/external/${CMAKE_CXX_COMPILER_ID}
#        -DTESTING=OFF
#        -DCOVERAGE=OFF
#        -DEXAMPLES=OFF
#        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
#        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
#        BUILD_COMMAND ${CMAKE_COMMAND} --build . --target install ${PARALLEL_ARG})

    # yaml-cpp
    set(V_YAML_CPP_URL ${baseDir}/package/yaml-cpp-0.7.0.zip)
    if(NOT EXISTS "${V_YAML_CPP_URL}")
        set(V_YAML_CPP_URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-0.7.0.zip)
    endif()
    ExternalProject_Add(ep_yaml-cpp
        URL ${V_YAML_CPP_URL}
        DOWNLOAD_NAME yaml-cpp-0.7.0.zip
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${baseDir}/external/${CMAKE_CXX_COMPILER_ID}
        -DYAML_BUILD_SHARED_LIBS=OFF
        -DYAML_CPP_BUILD_TESTS=OFF
        -DYAML_MSVC_SHARED_RT=OFF
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        BUILD_COMMAND ${CMAKE_COMMAND} --build . ${PARALLEL_ARG})

    # fmt
    set(V_FMT_URL ${baseDir}/package/fmt-7.1.2.zip)
    if(NOT EXISTS "${V_FMT_URL}")
        set(V_FMT_URL https://github.com/fmtlib/fmt/releases/download/7.1.2/fmt-7.1.2.zip)
    endif()
    ExternalProject_Add(ep_fmt
        URL ${V_FMT_URL}
        DOWNLOAD_NAME fmt-7.1.2.zip
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${baseDir}/external/${CMAKE_CXX_COMPILER_ID}
        -DFMT_DOC=OFF
        -DFMT_TEST=OFF
        -DFMT_FUZZ=OFF
        -DFMT_CUDA_TEST=OFF
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        BUILD_COMMAND ${CMAKE_COMMAND} --build . ${PARALLEL_ARG})

    # boost.di
#    set(V_BOOST_DI_URL ${baseDir}/package/boost-di-1.2.0.zip)
#    if(NOT EXISTS "${V_BOOST_DI_URL}")
#        set(V_BOOST_DI_URL https://github.com/boost-ext/di/archive/refs/tags/v1.2.0.zip)
#    endif()
#    ExternalProject_Add(ep_boost-di
#        URL ${V_BOOST_DI_URL}
#        DOWNLOAD_NAME boost-di-1.2.0.zip
#        CMAKE_ARGS
#        -DCMAKE_INSTALL_PREFIX:PATH=${baseDir}/external/${CMAKE_CXX_COMPILER_ID}
#        -DBOOST_DI_OPT_BUILD_TESTS=OFF
#        -DBOOST_DI_OPT_BUILD_EXAMPLES=OFF
#        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
#        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
#        BUILD_COMMAND ${CMAKE_COMMAND} --build . --target install ${PARALLEL_ARG})
endfunction()
