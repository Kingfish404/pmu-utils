#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pmu_utils.h>
#include <cache_utils.h>

#include <linux/perf_event.h>
#include <sys/ioctl.h>

uint8_t data_array[1024];

int test_pmu_utils()
{
    pmu_init();

    int core = 0; // no used in aarch64
    int pmu_id_0 = 0;
    int pmu_id_1 = 1;
    // Level 1 data cache refill
    int pmu_l1d_cache_refill = 0x3;
    // Branch instruction architecturally executed, immediate, taken
    int pmu_branch_inst_taken = 0x8108;

    data_array[0] = 1; // to avoid optimization, e.g., page not allocated
    data_array[512] = 2;
    data_array[1024] = 3;

    for (int i = 0; i < 1024; i++)
    {
        asm volatile("dc civac, %0\n" ::"r"((uint64_t)&data_array[i]) : "memory");
    }

    pmu_set_event(core, NULL, pmu_l1d_cache_refill, pmu_id_0);
    pmu_set_event(core, NULL, pmu_branch_inst_taken, pmu_id_1);

    uint64_t val_pmu_0_s, val_pmu_0_e;
    uint64_t val_pmu_1_s, val_pmu_1_e;
    register uint64_t val_s, val_e;
    usleep(100);
    asm volatile(
        "isb sy\n\t"
        "dsb sy\n\t"
        "mrs %0, PMCCNTR_EL0\n\t"
        "mrs %1, PMEVCNTR0_EL0\n\t"
        "mrs %2, PMEVCNTR1_EL0\n\t"
        "isb sy\n\t"
        "dsb sy\n\t"
        : "=r"(val_s), "=r"(val_pmu_0_s), "=r"(val_pmu_1_s)
        :
        : "memory");

    volatile register uint8_t tmp;
    for (int i = 0; i < 64 * 4; i++)
    {
        tmp += data_array[i];
    }

    asm volatile(
        "isb sy\n\t"
        "dsb sy\n\t"
        "mrs %0, PMCCNTR_EL0\n\t"
        "mrs %1, PMEVCNTR0_EL0\n\t"
        "mrs %2, PMEVCNTR1_EL0\n\t"
        "isb sy\n\t"
        "dsb sy\n\t"
        : "=r"(val_e), "=r"(val_pmu_0_e), "=r"(val_pmu_1_e)
        :
        : "memory");
    printf("Start count: %lu\n", val_s);
    printf("End count: %lu\n", val_e);
    printf("Delta: %lu\n", val_e - val_s);
    printf("L1D Cache Refill: %lu\n", val_pmu_0_e - val_pmu_0_s);
    printf("Branch Inst Taken: %lu\n", val_pmu_1_e - val_pmu_1_s);

    pmu_deinit();

    assert((val_pmu_1_e - val_pmu_1_s) > 0);
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
