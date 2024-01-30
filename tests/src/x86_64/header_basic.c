#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include "cache_utils.h"
#include "./../header/pmu_utils.h"
// #include "pmu.h"

char memory[1024];

int main(int argc, char const *argv[])
{
    uint64_t pmu_num = pmu_get_MSRs_num();
    printf("MSRs_num: \t%lu\n", pmu_num);
    uint64_t pmc_bit_width = pmu_get_PMC_bit_width();
    printf("PMC_bit_width: \t%lu\n", pmc_bit_width);

    int core = 0, pmu_id = 0;
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

    uint64_t pe0_code = pmu_event_to_hexcode(&pe0);
    printf("Event Hexcode: \t0x%lx\n", pe0_code);

    int msr_fd = pmu_open_msr(core);

    pmu_set_event(core, &msr_fd, pe0_code, pmu_id);
    pmu_set_msr_event(msr_fd, pe0_code, pmu_id);

    pmu_set_pmc(msr_fd, pmu_id, 0);
    pmu_record_start(msr_fd);
    uint64_t start_pmu_value = pmu_get_MSR_pmc(msr_fd, pmu_id);
    uint64_t start_pmu_value_rdpmc = pmu_get_rdpmc(pmu_id);
    asm volatile("mfence\n\t");

    for (int i = 0; i < sizeof(memory); i++)
    {
        flush(&memory[i]);
        asm volatile("mfence\n\t");
        maccess(&memory[i]);
    }

    asm volatile("mfence\n\t");
    uint64_t end_pmu_value = pmu_get_MSR_pmc(msr_fd, pmu_id);
    uint64_t end_pmu_value_rdpmc = pmu_get_rdpmc(pmu_id);
    pmu_record_stop(msr_fd);

    printf("MSR \tstart_pmu_value: %ld", start_pmu_value);
    printf("\tend_pmu_value: %ld\n", end_pmu_value);
    printf("RDPMC \tstart_pmu_value: %ld", start_pmu_value_rdpmc);
    printf("\tend_pmu_value: %ld\n", end_pmu_value_rdpmc);

    assert((start_pmu_value_rdpmc - start_pmu_value) < 10);
    assert((end_pmu_value_rdpmc - end_pmu_value) < 10);
    assert(end_pmu_value - start_pmu_value == end_pmu_value_rdpmc - start_pmu_value_rdpmc);
    assert(end_pmu_value - start_pmu_value > 0);
    return 0;
}
