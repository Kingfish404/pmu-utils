#include "pmu.h"

void write_to_IA32_PERFEVTSELi(int msr_fd, int i, uint64_t val)
{
    size_t ret;
    ret = pwrite(msr_fd, &val, sizeof(val), 0x186 + i);
    if (!ret)
        printf("Pwrite error!\n");
}

void write_to_IA32_PMCi(int msr_fd, int i, uint64_t val)
{
    size_t ret;
    ret = pwrite(msr_fd, &val, sizeof(val), 0xC1 + i);
    if (!ret)
        printf("Pwrite error!\n");
}

void write_to_IA32_PERF_GLOBAL_CTRL(int msr_fd, uint64_t val)
{
    size_t ret;
    ret = pwrite(msr_fd, &val, sizeof(val), 0x38F);
    if (!ret)
        printf("Pwrite error!\n");
}

uint64_t read_IA32_PMCi(int msr_fd, int i)
{
    uint64_t toret = -1;
    size_t ret;
    ret = pread(msr_fd, &toret, sizeof(toret), 0xC1 + i);
    if (!ret)
        printf("Pread error!\n");

    return toret;
}

uint64_t pmu_event_to_hexcode(PMU_EVENT *event)
{
    uint64_t hexcode = 0;
    hexcode |= event->event_code;
    hexcode |= event->umask << 8;
    hexcode |= event->user_mode << 16;
    hexcode |= event->os << 17;
    hexcode |= event->edge_detect << 18;
    hexcode |= event->pc << 19;
    hexcode |= event->int_enable << 20;
    hexcode |= event->enable_couters << 22;
    hexcode |= event->invert << 23;
    hexcode |= event->counter_mask << 24;
    return hexcode;
}

inline int pmu_open_msr(int core)
{
    core %= 512;
    char msr_path[32];
    sprintf(msr_path, "/dev/cpu/%d/msr", core);
    return open(msr_path, O_RDWR);
}

inline void pmu_set_event(int core, int *msr_fd, uint64_t hexcode, size_t pmu_id)
{
    core %= 512;
    char msr_path[32];
    sprintf(msr_path, "/dev/cpu/%d/msr", core);
    *msr_fd = open(msr_path, O_RDWR);
    /* DISABLE ALL COUNTERS */
    write_to_IA32_PERF_GLOBAL_CTRL(*msr_fd, 0ull);

    write_to_IA32_PERFEVTSELi(*msr_fd, pmu_id, hexcode);
    lseek(*msr_fd, 0x38F, SEEK_SET);
}

inline void pmu_set_msr_event(int msr_fd, uint64_t hexcode, size_t pmu_id)
{
    /* DISABLE ALL COUNTERS */
    write_to_IA32_PERF_GLOBAL_CTRL(msr_fd, 0ull);

    write_to_IA32_PERFEVTSELi(msr_fd, pmu_id, hexcode);
    lseek(msr_fd, 0x38F, SEEK_SET);
}

inline void pmu_set_pmc(int msr_fd, size_t pmu_id, uint64_t val)
{
    write_to_IA32_PMCi(msr_fd, pmu_id, val);
}

inline void pmu_record_start(int msr_fd)
{
    uint64_t val = 15ull | (7ull << 32);
    asm("mov %[write],     %%eax;"
        "mov %[fd],        %%edi;"
        "mov %[val],       %%rsi;"
        "mov $8,           %%edx;"
        "syscall;"
        :
        : [write] "i"(SYS_write),
          [val] "r"(&val),
          [fd] "m"(msr_fd)
        : "eax", "edi", "rsi", "edx");
}

inline void pmu_record_stop(int msr_fd)
{
    uint64_t val = 0;
    asm("mov %[write],     %%eax;"
        "mov %[fd],        %%edi;"
        "mov %[val],       %%rsi;"
        "mov $8,           %%edx;"
        "syscall;"
        :
        : [write] "i"(SYS_write),
          [val] "r"(&val),
          [fd] "m"(msr_fd)
        : "eax", "edi", "rsi", "edx");
}

inline uint64_t pmu_get_MSR_pmc(int msr_fd, size_t pmu_id)
{
    return read_IA32_PMCi(msr_fd, pmu_id);
}

uint64_t pmu_get_rdpmc(int pmu_id)
{
    uint32_t lo, hi;
    asm volatile(
        "mfence\n\t"
        "rdpmc\n\t"
        "mfence\n\t"
        : "=a"(lo), "=d"(hi)
        : "c"(pmu_id));
    return ((uint64_t)hi << 32 | lo);
}

uint64_t pmu_get_MSRs_num()
{
    uint32_t eax;
    asm volatile(
        "cpuid"
        : "=a"(eax)
        : "a"(0x0A)
        :);
    uint64_t num = (eax & 0xFF00) >> 8;
    return num;
}

uint64_t pmu_get_PMC_bit_width()
{
    uint32_t eax;
    asm volatile(
        "cpuid"
        : "=a"(eax)
        : "a"(0x0A)
        :);
    uint64_t bit_width = (eax & 0xFF0000) >> 16;
    return bit_width;
}
