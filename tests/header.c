#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include "./../header/cache_utils.h"
#include "./../header/pmu_utils.h"

int main(int argc, char const *argv[])
{
    PMU_EVENT pe0 = {
        .event_code = 0xD1,
        .umask = 0x08,
        .user_mode = 1,
        .os = 0,
        .edge_detect = 0,
        .pc = 0,
        .int_enable = 0,
        .enable_couters = 1,
        .invert = 0,
        .counter_mask = 0,
    }; // MEM_LOAD_RETIRED.L1_MISS

    uint64_t hexcode = pmu_event_to_hexcode(&pe0);

    int msr_fd = pmu_open_msr(0), core = 0, pmu_id = 0;

    pmu_set_event(core, &msr_fd, hexcode, pmu_id);

    pmu_set_msr_event(msr_fd, hexcode, pmu_id);

    pmu_set_pmc(msr_fd, pmu_id, 0);

    pmu_record_start(msr_fd);

    uint64_t pmu_val_msr = pmu_get_MSR_pmc(msr_fd, pmu_id);

    uint64_t pmu_val = pmu_get_rdpmc(pmu_id);

    uint64_t msrs_num = pmu_get_MSRs_num();

    uint64_t pmc_bit_width = pmu_get_PMC_bit_width();

    pmu_record_stop(msr_fd);

    printf("header all test function passed\n");
    printf("pmu_val_msr: %lu\n", pmu_val_msr);
    printf("pmu_val: %lu\n", pmu_val);
    printf("msrs_num: %lu\n", msrs_num);
    printf("pmc_bit_width: %lu\n", pmc_bit_width);

    return 0;
}
