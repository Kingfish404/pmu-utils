#define READ_PMU_N(pctr_arg, pmu_count, lo, hi) \
    __asm__ volatile(                           \
        "mfence\n\t"                            \
        "rdpmc\n\t"                             \
        "mfence\n\t"                            \
        : "=a"(lo), "=d"(hi)                    \
        : "c"(pctr_arg));                       \
    pmu_count = ((uint64_t)hi << 32 | lo);

#define READ_PMU_P(p, pctr_arg, pmu_count)    \
    __asm__ volatile("mfence\n\t"             \
                     "rdpmc\n\t"              \
                     "mfence\n\t"             \
                     "movl %%eax , %%esi\n\t" \
                                              \
                     "mov (%%rbx), %%rax\n\t" \
                                              \
                     "mfence\n\t"             \
                     "rdpmc\n\t"              \
                     "mfence\n\t"             \
                                              \
                     "sub %%esi, %%eax\n\t"   \
                     : "=a"(pmu_count)        \
                     : "c"(pctr_arg), "b"(p));

uint64_t read_pmu_0()
{
    uint32_t lo, hi;
    __asm__ volatile(
        "mfence\n\t"
        "rdpmc\n\t"
        "mfence\n\t"
        : "=a"(lo), "=d"(hi)
        : "c"(0x0));
    return ((uint64_t)hi << 32 | lo);
}

uint64_t read_pmu_1()
{
    uint32_t lo, hi;
    __asm__ volatile(
        "mfence\n\t"
        "rdpmc\n\t"
        "mfence\n\t"
        : "=a"(lo), "=d"(hi)
        : "c"(0x1));
    return ((uint64_t)hi << 32 | lo);
}

uint64_t read_pmu_2()
{
    uint32_t lo, hi;
    __asm__ volatile(
        "mfence\n\t"
        "rdpmc\n\t"
        "mfence\n\t"
        : "=a"(lo), "=d"(hi)
        : "c"(0x2));
    return ((uint64_t)hi << 32 | lo);
}

uint64_t read_pmu_3()
{
    uint32_t lo, hi;
    __asm__ volatile(
        "mfence\n\t"
        "rdpmc\n\t"
        "mfence\n\t"
        : "=a"(lo), "=d"(hi)
        : "c"(0x3));
    return ((uint64_t)hi << 32 | lo);
}

uint64_t read_pmu_n(const uint64_t n)
{
    uint32_t lo, hi;
    __asm__ volatile(
        "mfence\n\t"
        "rdpmc\n\t"
        "mfence\n\t"
        : "=a"(lo), "=d"(hi)
        : "c"(n));
    return ((uint64_t)hi << 32 | lo);
}
