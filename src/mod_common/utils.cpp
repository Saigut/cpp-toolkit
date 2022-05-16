#include <mod_common/utils.h>

#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>

#include <mod_common/log.h>


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
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * (uint64_t)1000000 + tv.tv_usec) / 1000;
}

void util_printf_buf(uint8_t* buf, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++) {
        printf("%02X ", buf[i]);
    }
    printf("\n");
}
