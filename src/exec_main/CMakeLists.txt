
set(V_program_name main)
add_executable(${V_program_name} exec_main.cpp)
target_link_libraries(${V_program_name}
    app_chat app_asio_socket app_socket
    app_im app_grpc_hw
    app_worker
    app_coroutine
    app_perf_test
    mod_coroutine
    mod_tcp_hole_punching
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
target_link_boost_libraries(${V_program_name} boost_thread boost_context boost_atomic boost_chrono boost_date_time
    boost_program_options)
target_link_libraries(${V_program_name} ${PLATFORM_LINK_LIB})
