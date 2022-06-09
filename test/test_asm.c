#include <stdint.h>
#include <stdio.h>
#include "../include/pmu.h"

int main(void)
{
    uint64_t count;
    int a = 10;
    for (int i = 0; i < 100; i++)
    {
        a += i;
    }
    READ_PMU(count);
    return 0;
}