#pragma once
#ifndef _LIBPMU_PMU_H
#define _LIBPMU_PMU_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

enum PMU_CPU_Vendor
{
    CPU_VENDER_UNCHECK,
    CPU_VENDOR_UNKNOWN,
    CPU_VENDOR_INTEL,
    CPU_VENDOR_AMD,
};

enum PMU_CPU_Vendor pmu_vender = CPU_VENDER_UNCHECK;

enum PMU_CPU_Vendor pmu_get_cpu_vendor(void)
{
    if (pmu_vender != CPU_VENDER_UNCHECK)
        return pmu_vender;

    char vendor_str[13];
    unsigned int eax, ebx, ecx, edx;
    asm volatile("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(0));
    memcpy(vendor_str, &ebx, 4);
    memcpy(vendor_str + 4, &edx, 4);
    memcpy(vendor_str + 8, &ecx, 4);
    if (strcmp(vendor_str, "GenuineIntel") == 0)
    {
        pmu_vender = CPU_VENDOR_INTEL;
    }
    else if (strcmp(vendor_str, "AuthenticAMD") == 0)
    {
        pmu_vender = CPU_VENDOR_AMD;
    }
    else
    {
        pmu_vender = CPU_VENDOR_INTEL;
    }
    return pmu_vender;
}

/*
    INTEL 64 and IA-32 Architectures Software Developer's Manual
    Figure 20-1. Layout of IA32_PERFEVTSELx MSRs
*/
typedef struct PMU_INTEL_EVENT_STRUCT
{
    uint32_t event_code;
    uint32_t umask;
    uint32_t user_mode;
    uint32_t os;
    uint32_t edge_detect;
    uint32_t pc;
    uint32_t int_enable;
    uint32_t enable_couters;
    uint32_t invert;
    uint32_t counter_mask;
} PMU_EVENT;

/*
    AMD64 Architecture Programmer’s Manual Volumes 1–5
    Figure 13-11. Core Performance Event-Select Register (PerfEvtSeln)
*/
typedef struct PMU_AMD_EVENT_STRUCT
{
    uint64_t event_select;
    uint32_t unit_mask;
    uint32_t user_mode;
    uint32_t os_mode;
    uint32_t edge_detect;
    uint32_t interrupt_enable;
    uint32_t counter_enable;
    uint32_t invert;
    uint32_t counter_mask;
    uint64_t hg_only;
} PMU_EVENT_AMD;

void write_to_x86_perf_eventi(int msr_fd, int i, uint64_t val)
{
    size_t ret;
    switch (pmu_get_cpu_vendor())
    {
    case CPU_VENDOR_INTEL:
        // 0x186 is the offset of IA32_PERFEVTSEL0 register
        ret = pwrite(msr_fd, &val, sizeof(val), 0x186 + i);
        break;
    case CPU_VENDOR_AMD:
        // C001_0000h is the offset of AMD64 PerfEvtSel0 register
        ret = pwrite(msr_fd, &val, sizeof(val), 0xC0010000 + i);
        break;
    default:
        break;
    }
    if (!ret)
    {
        printf("Pwrite error!\n");
    }
}

void write_to_x86_PMCi(int msr_fd, int i, uint64_t val)
{
    size_t ret;
    switch (pmu_get_cpu_vendor())
    {
    case CPU_VENDOR_INTEL:
        // 0xC1 is the offset of IA32_PMC0 register
        ret = pwrite(msr_fd, &val, sizeof(val), 0xC1 + i);
        break;
    case CPU_VENDOR_AMD:
        // C001_0004h is the offset of AMD64 PerfCtr0 register
        ret = pwrite(msr_fd, &val, sizeof(val), 0xC0010004 + i);
        break;
    default:
        break;
    }
    if (!ret)
    {
        printf("Pwrite error!\n");
    }
}

void write_to_IA32_PERF_GLOBAL_CTRL(int msr_fd, uint64_t val)
{
    size_t ret;
    ret = pwrite(msr_fd, &val, sizeof(val), 0x38F);
    if (!ret)
        printf("Pwrite error!\n");
}

uint64_t read_x86_PMCi(int msr_fd, int i)
{
    uint64_t toret = -1;
    size_t ret;
    switch (pmu_get_cpu_vendor())
    {
    case CPU_VENDOR_INTEL:
        ret = pread(msr_fd, &toret, sizeof(toret), 0xC1 + i);
        break;
    case CPU_VENDOR_AMD:
        ret = pread(msr_fd, &toret, sizeof(toret), 0xC0010004 + i);
    default:
        break;
    }
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

uint64_t pmu_amd_event_to_hexcode(PMU_EVENT_AMD *event)
{
    uint64_t hexcode = 0;
    hexcode |= event->event_select & 0xFF;
    hexcode |= (event->unit_mask & 0xFF) << 8;
    hexcode |= (event->user_mode & 0x1) << 16;
    hexcode |= (event->os_mode & 0x1) << 17;
    hexcode |= (event->edge_detect & 0x1) << 18;

    hexcode |= (event->interrupt_enable & 0x1) << 20;

    hexcode |= (event->counter_enable & 0x1) << 22;
    hexcode |= (event->invert & 0x1) << 23;
    hexcode |= (event->counter_mask & 0xFF) << 24;
    hexcode |= (event->event_select & 0xF00) << 32;
    hexcode |= (event->hg_only & 0x3) << 40;
    return hexcode;
}

int pmu_open_msr(int core)
{
    core %= 512;
    char msr_path[32];
    sprintf(msr_path, "/dev/cpu/%d/msr", core);
    return open(msr_path, O_RDWR);
}

void pmu_set_event(int core, int *msr_fd, uint64_t hexcode, size_t pmu_id)
{
    core %= 512;
    char msr_path[32];
    sprintf(msr_path, "/dev/cpu/%d/msr", core);
    *msr_fd = open(msr_path, O_RDWR);
    /* DISABLE ALL COUNTERS */
    write_to_IA32_PERF_GLOBAL_CTRL(*msr_fd, 0ull);

    write_to_x86_perf_eventi(*msr_fd, pmu_id, hexcode);
    lseek(*msr_fd, 0x38F, SEEK_SET);
}

void pmu_set_msr_event(int msr_fd, uint64_t hexcode, size_t pmu_id)
{
    /* DISABLE ALL COUNTERS */
    write_to_IA32_PERF_GLOBAL_CTRL(msr_fd, 0ull);

    write_to_x86_perf_eventi(msr_fd, pmu_id, hexcode);
    lseek(msr_fd, 0x38F, SEEK_SET);
}

void pmu_set_pmc(int msr_fd, size_t pmu_id, uint64_t val)
{
    write_to_x86_PMCi(msr_fd, pmu_id, val);
}

void pmu_record_start(int msr_fd)
{
    uint64_t val = 15ull | (7ull << 32);
    asm("mov %[write],     %%eax\n"
        "mov %[fd],        %%edi\n"
        "mov %[val],       %%rsi\n"
        "mov $8,           %%edx\n"
        "syscall\n"
        :
        : [write] "i"(SYS_write),
          [val] "r"(&val),
          [fd] "m"(msr_fd)
        : "eax", "edi", "rsi", "edx");
}

void pmu_record_stop(int msr_fd)
{
    uint64_t val = 0;
    asm("mov %[write],     %%eax\n"
        "mov %[fd],        %%edi\n"
        "mov %[val],       %%rsi\n"
        "mov $8,           %%edx\n"
        "syscall\n"
        :
        : [write] "i"(SYS_write),
          [val] "r"(&val),
          [fd] "m"(msr_fd)
        : "eax", "edi", "rsi", "edx");
}

uint64_t pmu_get_MSR_pmc(int msr_fd, size_t pmu_id)
{
    return read_x86_PMCi(msr_fd, pmu_id);
}

uint64_t pmu_get_rdpmc(int pmu_id)
{
    uint32_t lo, hi;
    asm volatile(
        "rdpmc\n"
        : "=a"(lo), "=d"(hi)
        : "c"(pmu_id));
    return ((uint64_t)hi << 32 | lo);
}

uint64_t pmu_get_MSRs_num()
{
    uint32_t reg;
    uint64_t num = 0;
    switch (pmu_get_cpu_vendor())
    {
    case CPU_VENDOR_INTEL:
        asm volatile(
            "cpuid"
            : "=a"(reg)
            : "a"(0x0A)
            :);
        num = (reg & 0xFF00) >> 8;
        break;
    case CPU_VENDOR_AMD:
        asm volatile(
            "cpuid"
            : "=c"(reg)
            : "a"(0x80000001)
            :);
        num = 4 * ((reg >> 24) && 0x1);
        num += 2 * ((reg >> 23) & 0x1);
        break;
    default:
        break;
    }
    return num;
}

uint64_t pmu_get_PMC_bit_width()
{
    uint32_t eax;
    uint64_t bit_width = 0;
    switch (pmu_get_cpu_vendor())
    {
    case CPU_VENDOR_INTEL:
        asm volatile(
            "cpuid"
            : "=a"(eax)
            : "a"(0x0A)
            :);
        bit_width = (eax & 0xFF0000) >> 16;
        break;
    case CPU_VENDOR_AMD:
        bit_width = 64;
    default:
        break;
    }
    return bit_width;
}

#define PMU_GET_RDPMC(pmu_id, lo, hi) \
    asm volatile(                     \
        "rdpmc\n"                     \
        : "=a"(lo), "=d"(hi)          \
        : "c"(pmu_id));

#endif /* _LIBPMU_PMU_H */
