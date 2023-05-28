#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include "cacheutils.h"
#include "./../libpmu/pmu.h"

int main(int argc, char const *argv[])
{
    uint64_t umask_and_event = 0x0;
    int core = 0, pmu_id = 0;
    if (argc >= 2)
    {
        core = (int)strtoull(argv[1], NULL, 0);
    }
    if (argc >= 3)
    {
        umask_and_event = strtoull(argv[2], NULL, 0);
    }
    PMU_EVENT pe0 = {
        .event_code = 0x00,
        .umask = 0x00,
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
    hexcode = umask_and_event | hexcode;

    int msr_fd = pmu_open_msr(core);

    pmu_set_event(core, &msr_fd, hexcode, pmu_id);

    pmu_set_msr_event(msr_fd, hexcode, pmu_id);

    pmu_set_pmc(msr_fd, pmu_id, 0);

    pmu_record_start(msr_fd);

    printf("setting pmu event to 0x%lx\n", hexcode);
    return 0;
}
