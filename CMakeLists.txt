# Prologue
cmake_minimum_required(VERSION 3.12)
project(cpp-toolkit C CXX)

# Include
set(GV_extra_dir extra)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${GV_extra_dir}/cmake")
include(common_var)
include(helper)
include(compiler)
#include(external_lib)

macro(setup_platform_environment)
    ## Setup compilation options
    compiler_set_compilation_options()
endmacro()

macro(setup_conan)
    find_program(V_conan_cmd conan)
    if (NOT V_conan_cmd)
        message(WARNING "conan is not found!" )
    else()
        message("Found conan: ${V_conan_cmd}")
        set(V_conan_gen_files
            "${CMAKE_BINARY_DIR}/build/generators/conan_toolchain.cmake")
        set(V_conanfile
            "${PROJECT_SOURCE_DIR}/${GV_extra_dir}/conanfile.txt")
        if (NOT EXISTS ${V_conanfile})
            message(WARNING "${V_conanfile} is not found!" )
        else()
            if (${V_conanfile} IS_NEWER_THAN ${V_conan_gen_files})
                file(REMOVE ${V_conan_gen_files})

                # Set up compiler settings
                set(V_conan_s_compiler "")
                set(V_conan_s_compiler_ver "")
                set(V_conan_s_compiler_cppstd "")
                set(V_conan_s_compiler_rt "")
                set(V_conan_s_build_type "")

                if (NOT V_conan_compiler STREQUAL "")
                    set(V_conan_s_compiler "-s:a" "compiler=${V_conan_compiler}")
                    set(V_conan_s_compiler_ver "-s:a" "compiler.version=${CMAKE_CXX_COMPILER_VERSION_MAJOR}")

                    if (V_conan_compiler STREQUAL "msvc")
                        # Handle MSVC-specific version formatting
                        string(SUBSTRING "${CMAKE_CXX_COMPILER_VERSION_MINOR}" 0 1 V_tmp_number)
                        set(V_conan_s_compiler_ver "-s:a" "compiler.version=${CMAKE_CXX_COMPILER_VERSION_MAJOR}${V_tmp_number}")
                        set(V_conan_s_compiler_cppstd "-s:a" "compiler.cppstd=${CMAKE_CXX_STANDARD}")
                        set(V_conan_s_compiler_rt "-s:a" "compiler.runtime=dynamic")
                    elseif (NOT "${CMAKE_CXX_COMPILER_VERSION_MINOR}" STREQUAL "")
                        # Optionally handle non-MSCV compilers if needed
                    endif()
                endif()

                # Set up build type
                if (CMAKE_BUILD_TYPE STREQUAL "Debug")
                    set(V_conan_s_build_type "-sbuild_type=Debug")
                else()
                    set(V_conan_s_build_type "-sbuild_type=Release")
                endif()

                # Prepare the conan install command
                set(V_conan_cmd "D:/Program Files/Python313/Scripts/conan.exe")
                set(V_conan_args install
                    "${V_conanfile}"
                    ${V_conan_s_compiler}
                    ${V_conan_s_compiler_ver}
                    ${V_conan_s_compiler_cppstd}
                    ${V_conan_s_compiler_rt}
                    ${V_conan_s_build_type}
                    "--build=missing"
                    "-of=${CMAKE_BINARY_DIR}"
                    )

                # Debug: Show the constructed command
                string(REPLACE ";" " " V_conan_cmd_str "${V_conan_cmd} ${V_conan_args}")
                message("Executing: ${V_conan_cmd_str}")

                # Execute the conan install command
                execute_process(
                    COMMAND ${V_conan_cmd} ${V_conan_args}
                    RESULT_VARIABLE result
                    OUTPUT_VARIABLE output
                    ERROR_VARIABLE error
                )

                # Debug: Check the results
                if (result)
                    message(FATAL_ERROR "Conan install failed: ${error}")
                else()
                    message("Conan install succeeded: ${output}")
                endif()
            endif()
        endif()
    endif()
endmacro()

macro(setup_build_environment)
    ## Setup Conan
#    conan_basic_setup()

    setup_conan()
#    add_definitions(${CONAN_DEFINES})

    find_package(gRPC REQUIRED)
    find_package(Boost REQUIRED)
    find_package(ftxui REQUIRED)
    find_package(json-c REQUIRED)
    find_package(httplib REQUIRED)
    find_package(nlohmann_json REQUIRED)
    find_package(OpenSSL REQUIRED)

    ## Setup product options
    ### set_product_options()

    # Paths
    set(GV_external_dir ${GV_extra_dir}/external/${CMAKE_CXX_COMPILER_ID})
    message("External library path: ${GV_external_dir}")
    get_filename_component(GV_external_dir_a ${GV_external_dir} ABSOLUTE)
    ## Headers
    include_directories(
        include
        ${GV_extra_dir}/gen/grpc/cpp
        ${absl_INCLUDE_DIRS}
        ${protobuf_INCLUDE_DIRS}
        ${gRPC_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${ftxui_INCLUDE_DIRS}
        ${json-c_INCLUDE_DIRS}
        ${httplib_INCLUDE_DIRS}
        ${nlohmann_json_INCLUDE_DIRS}
#        ${GV_external_dir_a}/include
    )
    ## Libraries
    link_directories(
        ${GV_extra_dir}/gen/lib
#        ${GV_external_dir_a}/lib
    )
    ## Output
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endmacro()

macro(setup_directries)
    set(V_dir_gen ${PROJECT_SOURCE_DIR}/${GV_extra_dir}/gen)
    set(V_dir_gen_grpc ${V_dir_gen}/grpc)
    set(V_dir_gen_grpc_cpp ${V_dir_gen}/grpc/cpp)
    set(V_dir_gen_lib ${V_dir_gen}/lib)
    add_custom_command(
        OUTPUT "${V_dir_gen}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${V_dir_gen}
        DEPENDS "${PROJECT_SOURCE_DIR}/${GV_extra_dir}"
    )
    add_custom_target(GT_dir_gen DEPENDS "${V_dir_gen}")
    add_custom_command(
        OUTPUT "${V_dir_gen_grpc}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${V_dir_gen_grpc}
        DEPENDS GT_dir_gen
    )
    add_custom_target(GT_dir_gen_grpc DEPENDS "${V_dir_gen_grpc}")
    add_custom_command(
        OUTPUT "${V_dir_gen_grpc_cpp}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${V_dir_gen_grpc_cpp}
        DEPENDS GT_dir_gen_grpc
    )
    add_custom_target(GT_dir_gen_grpc_cpp DEPENDS "${V_dir_gen_grpc_cpp}")
    add_custom_command(
        OUTPUT "${V_dir_gen_lib}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${V_dir_gen_lib}
        DEPENDS GT_dir_gen
    )
    add_custom_target(GT_dir_gen_lib DEPENDS "${V_dir_gen_lib}")
endmacro()

macro(setup_grpc_target)
    # Target T_gen_grpc, for every proto file, generate code to gen/pb/
    set(V_dir_grpc_proto_r ${GV_extra_dir}/code_to_gen/protos)
    get_filename_component(V_dir_grpc_proto_a ${V_dir_grpc_proto_r} ABSOLUTE)
    file(GLOB V_all_proto_files "${V_dir_grpc_proto_r}/*.proto")

    unset(V_ptotoc_cmd CACHE)
    get_target_property(V_ptotoc_cmd protobuf::protoc IMPORTED_LOCATION)
    if (NOT V_ptotoc_cmd)
        message(FATAL_ERROR "protoc is not found!" )
    endif()
    message("Found protoc: ${V_ptotoc_cmd}")
    unset(V_ptotoc_cpp_plugin CACHE)
    get_target_property(V_ptotoc_cpp_plugin gRPC::grpc_cpp_plugin IMPORTED_LOCATION)
    if (NOT V_ptotoc_cpp_plugin)
        message(FATAL_ERROR "grpc_cpp_plugin is not found!")
    endif()
    message("Found grpc_cpp_plugin: ${V_ptotoc_cpp_plugin}")

    foreach(one_proto_file ${V_all_proto_files})
        get_filename_component(V_bname ${one_proto_file} NAME_WE)
        list(APPEND GV_proto_bnames ${V_bname})

        set(V_gen_cpp_grpc_srcs
            "${V_dir_gen_grpc_cpp}/${V_bname}.pb.h"
            "${V_dir_gen_grpc_cpp}/${V_bname}.pb.cc"
            "${V_dir_gen_grpc_cpp}/${V_bname}.grpc.pb.h"
            "${V_dir_gen_grpc_cpp}/${V_bname}.grpc.pb.cc"
            )
        list(APPEND GV_gen_cpp_grpc_srcs ${V_gen_cpp_grpc_srcs})

        add_custom_command(
            OUTPUT ${V_gen_cpp_grpc_srcs}
            COMMAND ${V_ptotoc_cmd}
            ARGS --grpc_out "${V_dir_gen_grpc_cpp}"
                --cpp_out "${V_dir_gen_grpc_cpp}"
                -I "${V_dir_grpc_proto_a}"
                --plugin=protoc-gen-grpc=${V_ptotoc_cpp_plugin}
                ${one_proto_file}
            DEPENDS "${one_proto_file}" GT_dir_gen_grpc_cpp
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
    set(GV_cpp_toolkit_src "")
    get_filename_component(GV_src_path "src" ABSOLUTE)
    file(GLOB GV_all_mod "${GV_src_path}/mod_*")
    file(GLOB GV_all_app "${GV_src_path}/app_*")
    file(GLOB GV_all_exec "${GV_src_path}/exec_*")

    setup_directries()
    setup_grpc_target()

    # Modules library
    foreach(one_dir ${GV_all_mod} ${GV_all_app})
        get_filename_component(tmp_name ${one_dir} NAME)
        file(GLOB GV_${tmp_name}_src "${one_dir}/*.cpp" "${one_dir}/*.c")
        file(GLOB tmp_sub_dirs "${one_dir}/*")
        foreach(one_sub_dir ${tmp_sub_dirs})
            if(IS_DIRECTORY ${one_sub_dir})
                file(GLOB tmp_sub_dir_files "${one_sub_dir}/*.cpp" "${one_sub_dir}/*.c")
                list(APPEND GV_${tmp_name}_src ${tmp_sub_dir_files})
            endif()
        endforeach()
        list(APPEND GV_cpp_toolkit_src ${GV_${tmp_name}_src})
#        add_library(${tmp_name} STATIC ${GV_${tmp_name}_src})
#        set_target_properties(${tmp_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${V_dir_gen_lib}")
#        add_dependencies(${tmp_name} GT_gen_grpc_src GT_dir_gen_lib)
    endforeach()

    # Library in one
    add_library(cpp_toolkit STATIC ${GV_cpp_toolkit_src})
    set_target_properties(cpp_toolkit PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${V_dir_gen_lib}")
    add_dependencies(cpp_toolkit GT_gen_grpc_src GT_dir_gen_lib)

    # Main executable
    foreach(one_dir ${GV_all_exec})
        add_subdirectory(${one_dir})
    endforeach()
endmacro()

macro(main_func)
    setup_platform_environment()
    setup_build_environment()
    setup_targets()
endmacro()

# TODO   Write utils.cmake, put some useful general functions

main_func()
