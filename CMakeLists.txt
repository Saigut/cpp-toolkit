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
set(V_main_src main.cpp)


# Product
## Targets and properties
add_executable(main ${V_main_src})
target_link_libraries(main json-c)

add_dependencies(main ep_json-c ep_boost)

# Test
