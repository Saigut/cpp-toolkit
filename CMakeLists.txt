# Prologue
cmake_minimum_required(VERSION 3.0)
project(cpp-toolkit C CXX)

# Prepare
## Cmake initial options
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

## Import other cmake. Vairables and functions etc.
include(common_var)
include(helper)
include(compiler)
include(external_lib)

## Setup dependency libraries
external_lib_setup()

## Setup compilation options
compiler_set_compilation_options()

## Setup product options
### set_product_options()


# Compile
## Headers for some targets
include_directories(
    include
    external/include
    external/include/boost-1_72
)

## Libraries for some targets
link_directories(
    external/lib
)

## Sources for some targets
set(GV_main_src main.cpp src/program_main.cpp)

get_filename_component(GV_src_path "src" ABSOLUTE)
file(GLOB GV_all_mod "${GV_src_path}/mod_*")
file(GLOB GV_all_app "${GV_src_path}/app_*")
foreach(one_dir ${GV_all_mod} ${GV_all_app})
    get_filename_component(tmp_name ${one_dir} NAME)
    file(GLOB GV_${tmp_name}_src "${one_dir}/*.cpp")
    add_library(${tmp_name} STATIC ${GV_${tmp_name}_src})
    set_target_properties(${tmp_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen/lib")
endforeach()


# Product
## Targets and properties
add_executable(main ${GV_main_src})
target_link_libraries(main app_chat app_socket mod_socket json-c)
target_link_boost_libraries(main boost_atomic boost_chrono boost_date_time)

add_dependencies(main ep_json-c ep_boost)

# Test
