#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <memory.h>
#include <sys/mman.h>
#include <linux/mman.h>
#include "../include/pmu.h"

void flush(void *p)
{
    asm volatile("clflush 0(%0)\n"
                 :
                 : "c"(p)
                 : "rax");
}

void maccess(void *p)
{
    asm volatile("movq (%0),%%rax"
                 :
                 : "c"(p)
                 : "rax");
}

int main(void)
{
    char *mapping = (char *)mmap(0, 15 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    memset(mapping, 0, 4096);

    uint32_t count = 1000;
    uint64_t sum = 0, pmu_count, pctr_arg = 0x0;
    for (uint32_t i = 0; i < count; i++)
    {
        flush(mapping);
        READ_PMU_P(mapping, pctr_arg, pmu_count);
        // pmu_count = read_pmu_p(mapping);
        sum += pmu_count;
    }
    printf("pmu_count:%ld\n", sum);
    return 0;
}
