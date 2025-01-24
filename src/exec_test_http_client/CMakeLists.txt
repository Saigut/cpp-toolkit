
set(V_program_name test_http_client)
add_executable(${V_program_name} exec_main.cpp)
target_link_libraries(${V_program_name} cpp_toolkit ${PLATFORM_LINK_LIB})
