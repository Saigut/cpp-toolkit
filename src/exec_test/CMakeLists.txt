
set(V_program_name test)
add_executable(${V_program_name} exec_test.cpp)
target_link_libraries(${V_program_name} cpp_toolkit ${PLATFORM_LINK_LIB})
