#include <stdio.h>
#include <pmu_utils.h>
#include <cache_utils.h>

#include <string.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>

int test_pmu_utils()
{
    pmu_init();
    register uint64_t val_s, val_e;
    asm volatile(
        "isb\n\t"
        "dsb sy\n\t"
        "mrs %0, PMCCNTR_EL0\n\t"
        "isb\n\t"
        "dsb sy\n\t"
        : "=r"(val_s)
        :
        : "memory");

    asm volatile(
        "isb\n\t"
        "dsb sy\n\t"
        "mrs %0, PMCCNTR_EL0\n\t"
        "isb\n\t"
        "dsb sy\n\t"
        : "=r"(val_e)
        :
        : "memory");
    printf("Start count: %lu\n", val_s);
    printf("End count: %lu\n", val_e);
    printf("Delta: %lu\n", val_e - val_s);
    pmu_deinit();
    return 0;
}

int test_pref()
{
    perf_init();

    long long start_count, end_count;
    read(perf_fd, &start_count, sizeof(long long));

    read(perf_fd, &end_count, sizeof(long long));
    printf("Start count: %lld\n", start_count);
    printf("End count: %lld\n", end_count);
    printf("Delta: %lld\n", end_count - start_count);

    perf_deinit();
    return 0;
}

int main(int argc, char const *argv[])
{
    printf("Testing PMU utils:\n");
    test_pmu_utils();
    printf("Testing perf event:\n");
    test_pref();
    return 0;
}
