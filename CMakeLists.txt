# Prologue
cmake_minimum_required(VERSION 3.0)
project(cpp-toolkit C CXX)

# Include
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
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
    ## Headers
    include_directories(
        include
        external/${CMAKE_CXX_COMPILER_ID}/include
        external/${CMAKE_CXX_COMPILER_ID}/include/boost
    )
    ## Libraries
    link_directories(
        external/${CMAKE_CXX_COMPILER_ID}/lib
    )
endmacro()

macro(setup_targets)
    set(GV_main_src main.cpp src/program_main.cpp)
    get_filename_component(GV_src_path "src" ABSOLUTE)
    file(GLOB GV_all_mod "${GV_src_path}/mod_*")
    file(GLOB GV_all_app "${GV_src_path}/app_*")

    # Modules library
    foreach(one_dir ${GV_all_mod} ${GV_all_app})
        get_filename_component(tmp_name ${one_dir} NAME)
        file(GLOB GV_${tmp_name}_src "${one_dir}/*.cpp")
        add_library(${tmp_name} STATIC ${GV_${tmp_name}_src})
        set_target_properties(${tmp_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen/lib")
    endforeach()

    # Main executable
    add_executable(main ${GV_main_src})
    target_link_libraries(main app_chat app_asio_socket app_socket mod_socket json-c)
    target_link_boost_libraries(main boost_atomic boost_chrono boost_date_time)
    target_link_libraries(main ${PLATFORM_LINK_LIB})
endmacro()


macro(main)
    setup_platform_environment()
    setup_build_environment()
    setup_targets()
endmacro()


main()
