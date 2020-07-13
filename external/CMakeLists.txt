# Prologue
cmake_minimum_required(VERSION 3.0)
project(cpp-toolkit-external C CXX)

# Prepare
## Cmake initial options
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

## Import other cmake. Vairables and functions etc.
include(common_var)
include(helper)
include(compiler)
include(external_lib)

## Setup compilation options
compiler_set_compilation_options()

## Setup dependency libraries
external_lib_setup(${PROJECT_SOURCE_DIR}/..)

add_custom_target(ExternalLibs ALL DEPENDS ep_json-c ep_boost)
