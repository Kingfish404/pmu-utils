#include <stdio.h>
#include <pmu_utils.h>
#include <cache_utils.h>

int main(int argc, char const *argv[])
{
    cache_init();
    // for ARM Neoverse N2
    // 0x01, L1I_CACHE_REFILL, L1 instruction cache refill
    // 0x14, L1I_CACHE, Level 1 instruction cache access
    // 0x03, L1D_CACHE_REFILL, L1 data cache refill
    // 0x04, L1D_CACHE, L1 data cache access
    uint64_t evet_code[2] = {0x03, 0x04};
    uint64_t pmu_values[2];
    pmu_set_event(0x0, NULL, evet_code[0], 0);
    pmu_set_event(0x0, NULL, evet_code[0], 1);
    flush(mem);

    pmu_values[0] = pmu_get_rdpmc(0);
    pmu_values[1] = pmu_get_rdpmc(1);
    for (int i = 0; i < 1000; i++)
    {
        if (i % 10 == 0)
        {
            flush(mem);
        }
        mfence();
        maccess(mem);
    }
    pmu_values[0] = pmu_get_rdpmc(0) - pmu_values[0];
    pmu_values[1] = pmu_get_rdpmc(1) - pmu_values[1];
    printf("pmu_values[0]: %lu\n", pmu_values[0]);
    printf("pmu_values[1]: %lu\n", pmu_values[1]);
    assert((pmu_values[0] + pmu_values[1]) != 0);
    return 0;
}
