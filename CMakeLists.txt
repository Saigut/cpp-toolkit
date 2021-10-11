# Prologue
cmake_minimum_required(VERSION 3.12)
project(cpp-toolkit C CXX)

# Include
set(GV_other_dir z_other)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${GV_other_dir}/cmake")
include(common_var)
include(helper)
include(compiler)
#include(external_lib)

macro(setup_platform_environment)
    ## Setup compilation options
    compiler_set_compilation_options()
endmacro()

macro(setup_build_environment)
    ## Setup dependency libraries
    #external_lib_setup(${PROJECT_SOURCE_DIR})

    ## Setup product options
    ### set_product_options()

    # Paths
    set(GV_external_dir ${GV_other_dir}/external/${CMAKE_CXX_COMPILER_ID})
    get_filename_component(GV_external_dir_a ${GV_external_dir} ABSOLUTE)
    ## Headers
    include_directories(
        include
        ${GV_other_dir}/gen/grpc/cpp
        ${GV_external_dir}/include
        ${GV_external_dir}/include/boost
    )
    ## Libraries
    link_directories(
        ${GV_external_dir}/lib
        ${GV_other_dir}/gen/lib
    )
endmacro()

macro(setup_directries)
    set(V_dir_gen ${PROJECT_SOURCE_DIR}/${GV_other_dir}/gen)
    set(V_dir_gen_grpc ${V_dir_gen}/grpc/cpp)
    set(V_dir_gen_lib ${V_dir_gen}/lib)
    add_custom_command(
        OUTPUT "${V_dir_gen}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${V_dir_gen}
    )
    add_custom_command(
        OUTPUT "${V_dir_gen_grpc}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${V_dir_gen_grpc}
        DEPENDS "${V_dir_gen}"
    )
    add_custom_target(GT_dir_gen_grpc DEPENDS "${V_dir_gen_grpc}")
    add_custom_command(
        OUTPUT "${V_dir_gen_lib}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${V_dir_gen_lib}
        DEPENDS "${V_dir_gen}"
    )
    add_custom_target(GT_dir_gen_lib DEPENDS "${V_dir_gen_lib}")
endmacro()

macro(setup_grpc_target)
    # Target T_gen_grpc, for every proto file, generate code to gen/pb/
    set(V_dir_grpc_proto_r ${GV_other_dir}/code_to_gen/protos)
    get_filename_component(V_dir_grpc_proto_a ${V_dir_grpc_proto_r} ABSOLUTE)
    file(GLOB V_all_proto_files "${V_dir_grpc_proto_r}/*.proto")

    foreach(one_proto_file ${V_all_proto_files})
        get_filename_component(V_bname ${one_proto_file} NAME_WE)
        list(APPEND GV_proto_bnames ${V_bname})

        set(V_gen_cpp_grpc_srcs
            "${V_dir_gen_grpc}/${V_bname}.pb.h"
            "${V_dir_gen_grpc}/${V_bname}.pb.cc"
            "${V_dir_gen_grpc}/${V_bname}.grpc.pb.h"
            "${V_dir_gen_grpc}/${V_bname}.grpc.pb.cc"
            )
        list(APPEND GV_gen_cpp_grpc_srcs ${V_gen_cpp_grpc_srcs})

        if(OS_WINDOWS)
            set(V_ptotoc_cmd ${GV_external_dir_a}/bin/protoc.exe)
            set(V_ptotoc_cpp_plugin ${GV_external_dir_a}/bin/grpc_cpp_plugin.exe)
        else()
            set(V_ptotoc_cmd ${GV_external_dir_a}/bin/protoc)
            set(V_ptotoc_cpp_plugin ${GV_external_dir_a}/bin/grpc_cpp_plugin)
        endif()

        add_custom_command(
            OUTPUT ${V_gen_cpp_grpc_srcs}
            COMMAND ${V_ptotoc_cmd}
            ARGS --grpc_out "${V_dir_gen_grpc}"
                --cpp_out "${V_dir_gen_grpc}"
                -I "${V_dir_grpc_proto_a}"
                --plugin=protoc-gen-grpc=${V_ptotoc_cpp_plugin}
                ${one_proto_file}
            DEPENDS "${one_proto_file}" GT_dir_gen_grpc
        )
        add_custom_target(GT_gen_${V_bname}_grpc_cpp_src DEPENDS ${V_gen_cpp_grpc_srcs})
        list(APPEND GV_gen_grpc_cpp_src_targets GT_gen_${V_bname}_grpc_cpp_src)
    endforeach()

    add_custom_target(GT_gen_grpc_src DEPENDS ${GV_gen_grpc_cpp_src_targets})

    set(GV_gen_grpc_cpp_lib_bname gen_grpc_cpp_src)
    add_library(${GV_gen_grpc_cpp_lib_bname} STATIC ${GV_gen_cpp_grpc_srcs})
    add_dependencies(${GV_gen_grpc_cpp_lib_bname} GT_gen_grpc_src)
    set_target_properties(${GV_gen_grpc_cpp_lib_bname} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${V_dir_gen_lib}")
    set_target_properties(${GV_gen_grpc_cpp_lib_bname} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${V_dir_gen_lib}")
endmacro()

macro(setup_targets)
    set(GV_main_src main.cpp src/program_main.cpp)
    get_filename_component(GV_src_path "src" ABSOLUTE)
    file(GLOB GV_all_mod "${GV_src_path}/mod_*")
    file(GLOB GV_all_app "${GV_src_path}/app_*")

    setup_directries()
    setup_grpc_target()

    if (${OS_WINDOWS})
        set(V_PROTO_libs
            libprotobuf-lited
            libprotobufd
            libprotocd)
        set(V_LIBZ_libs
            zlibstaticd)
    else()
        set(V_PROTO_libs
            protobuf
            protoc)
        set(V_LIBZ_libs
            z)
    endif ()

    # Modules library
    foreach(one_dir ${GV_all_mod} ${GV_all_app})
        get_filename_component(tmp_name ${one_dir} NAME)
        file(GLOB GV_${tmp_name}_src "${one_dir}/*.cpp" "${one_dir}/*.c")
        add_library(${tmp_name} STATIC ${GV_${tmp_name}_src})
        set_target_properties(${tmp_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${V_dir_gen_lib}")
        add_dependencies(${tmp_name} GT_gen_grpc_src GT_dir_gen_lib)
    endforeach()

    # Main executable
    add_executable(main ${GV_main_src})
    target_link_libraries(main
            app_chat app_asio_socket app_socket
            app_im app_grpc_hw
            app_worker
            mod_worker
            mod_socket
            mod_hash_table mod_queue
            mod_common
            grpc++_reflection
            grpc++
            grpc
            gpr
            ${V_PROTO_libs}

            address_sorting
            upb
            re2
            cares
            ${V_LIBZ_libs}

            absl_synchronization
            absl_raw_logging_internal
            absl_spinlock_wait
            absl_time
            absl_time_zone
            absl_int128
            absl_cord
            absl_strings
            absl_strings_internal
            absl_str_format_internal
            absl_stacktrace
            absl_symbolize
            absl_status
            absl_city
            absl_civil_time
            absl_bad_optional_access
            absl_bad_variant_access
            absl_debugging_internal
            absl_demangle_internal
            absl_exponential_biased
            absl_graphcycles_internal
            absl_hash
            absl_raw_hash_set
            absl_hashtablez_sampler
            absl_base
            absl_throw_delegate
            absl_malloc_internal

            ssl
            crypto

            json-c ${GV_gen_grpc_cpp_lib_bname})
    target_link_boost_libraries(main boost_atomic boost_chrono boost_date_time boost_context)
    target_link_libraries(main ${PLATFORM_LINK_LIB})
endmacro()

macro(main_func)
    setup_platform_environment()
    setup_build_environment()
    setup_targets()
endmacro()

# TODO   Write utils.cmake, put some useful general functions

main_func()
