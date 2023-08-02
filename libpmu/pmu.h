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

uint64_t pmu_event_to_hexcode(PMU_EVENT *event);

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
