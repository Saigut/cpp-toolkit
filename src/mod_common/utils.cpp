#include <mod_common/utils.h>

#include <stdint.h>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
    #include <processthreadsapi.h>
#else
    #include <pthread.h>
    #include <unistd.h>
    #include <sys/time.h>
#endif


int util_bind_thread_to_core(int core_id)
{
#ifdef _WIN32
    if (0 == SetThreadAffinityMask(GetCurrentThread(), 1 << core_id)) {
        return -1;
    } else {
        return 0;
    }
#else
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id < 0 || core_id >= num_cores) {
        return -1;
    }
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    if (0 != pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
        return -1;
    } else {
        return 0;
    }
#endif
}

uint64_t util_now_ts_ms()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t util_now_ts_us()
{
    using namespace std::chrono;
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t util_now_ts_ns()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

void util_printf_buf(uint8_t* buf, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++) {
        printf("%02X ", buf[i]);
    }
    printf("\n");
}

void cppt_msleep(unsigned ts_ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ts_ms));
}

void cppt_usleep(unsigned ts_us)
{
    std::this_thread::sleep_for(std::chrono::microseconds(ts_us));
}

void cppt_nanosleep(unsigned ts_ns)
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(ts_ns));
}
