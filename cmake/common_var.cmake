include_guard()

# About compiler
set(CLANG false)
set(GCC false)
set(INTEL false)
set(MSVC false)

# About OS
set(OS_LINUX false)
set(OS_WINDOWS false)
set(OS_DARWIN false)

# Logic cores
cmake_host_system_information(RESULT LCORES QUERY NUMBER_OF_LOGICAL_CORES)
