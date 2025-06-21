#pragma once
#ifndef _LIBPMU_PMU_H
#define _LIBPMU_PMU_H

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>

enum PMU_CPU_Vendor
{
    CPU_VENDER_UNCHECK,
    CPU_VENDOR_UNKNOWN,
    CPU_VENDOR_INTEL,
    CPU_VENDOR_AMD,
    CPU_VENDOR_ARM,
    CPU_VENDOR_RISCV
};

/*
    INTEL 64 and IA-32 Architectures Software Developer's Manual
    Figure 20-1. Layout of IA32_PERFEVTSELx MSRs
*/
typedef struct PMU_EVENT_STRUCT
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

void perf_init();

uint64_t pmu_event_to_hexcode(PMU_EVENT *event);

uint64_t pmu_amd_event_to_hexcode(PMU_EVENT_AMD *event);

int pmu_open_msr(int core);

void pmu_set_event(int core, int *msr_fd, uint64_t hexcode, size_t pmu_id);

void pmu_set_msr_event(int msr_fd, uint64_t hexcode, size_t pmu_id);

void pmu_set_pmc(int msr_fd, size_t pmu_id, uint64_t val);

void pmu_record_start(int msr_fd);

void pmu_record_stop(int msr_fd);

uint64_t pmu_get_MSR_pmc(int msr_fd, size_t pmu_id);

uint64_t pmu_get_rdpmc(int pmu_id);

uint64_t pmu_get_MSRs_num();

uint64_t pmu_get_PMC_bit_width();

#define PMU_GET_RDPMC(pmu_id, lo, hi) \
    asm volatile(                     \
        "mfence\n\t"                  \
        "rdpmc\n\t"                   \
        "mfence\n\t"                  \
        : "=a"(lo), "=d"(hi)          \
        : "c"(pmu_id));

#endif /* _LIBPMU_PMU_H */
