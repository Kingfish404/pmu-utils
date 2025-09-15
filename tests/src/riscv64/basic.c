#include <stdio.h>
#include <pmu_utils.h>

int main()
{
    uint64_t val_s, val_e;
    pmu_init();
    asm volatile(
        "fence\n\t"
        "rdcycle %0\n\t"
        : "=r"(val_s)
        :
        : "memory");
    // Code region to be measured
    for (volatile int i = 0; i < 1000; i++)
    {
        asm volatile("nop");
    }
    asm volatile(
        "fence\n\t"
        "rdcycle %0\n\t"
        : "=r"(val_e)
        :
        : "memory");
    printf("Delta: %lu\n", val_e - val_s);
    pmu_deinit();
    return 0;
}