#include "include/msrdrv.h"
#include "include/pmu_event.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

static int loadDriver()
{
    int fd;
    fd = open("/dev/" DEV_NAME, O_RDWR);
    if (fd == -1)
    {
        perror("Failed to open /dev/" DEV_NAME);
    }
    return fd;
}

static void closeDriver(int fd)
{
    int e;
    e = close(fd);
    if (e == -1)
    {
        perror("Failed to close fd");
    }
}

/*
 * Reference:
 * Intel Software Developer's Manual Vol 3B "253669.pdf" August 2012
 * Intel Software Developer's Manual Vol 3C "326019.pdf" August 2012
 */
int main(int argc, char *argv[])
{
    unsigned long long pe0, pe1, pe2, pe3;
    if (argc == 5)
    {
        pe0 = atoll(argv[1]);
        pe1 = atoll(argv[2]);
        pe2 = atoll(argv[3]);
        pe3 = atoll(argv[4]);
    }
    else
    {
        pe0 = PMC0_EVENT;
        pe1 = PMC1_EVENT;
        pe2 = PMC2_EVENT;
        pe3 = PMC3_EVENT;
    }
    int fd;

    struct MsrInOut msr_start[] = {
        {MSR_WRITE, 0x38f, 0x00, 0x00},  // ia32_perf_global_ctrl: disable 4 PMCs & 3 FFCs
        {MSR_WRITE, 0xc1, 0x00, 0x00},   // ia32_pmc0: zero value (35-5)
        {MSR_WRITE, 0xc2, 0x00, 0x00},   // ia32_pmc1: zero value (35-5)
        {MSR_WRITE, 0xc3, 0x00, 0x00},   // ia32_pmc2: zero value (35-5)
        {MSR_WRITE, 0xc4, 0x00, 0x00},   // ia32_pmc3: zero value (35-5)
        {MSR_WRITE, 0x309, 0x00, 0x00},  // ia32_fixed_ctr0: zero value (35-17)
        {MSR_WRITE, 0x30a, 0x00, 0x00},  // ia32_fixed_ctr1: zero value (35-17)
        {MSR_WRITE, 0x30b, 0x00, 0x00},  // ia32_fixed_ctr2: zero value (35-17)
        {MSR_WRITE, 0x186, pe0, 0x00},   // ia32_perfevtsel1
        {MSR_WRITE, 0x187, pe1, 0x00},   // ia32_perfevtsel0
        {MSR_WRITE, 0x188, pe2, 0x00},   // ia32_perfevtsel2
        {MSR_WRITE, 0x189, pe3, 0x00},   // ia32_perfevtsel3
        {MSR_WRITE, 0x38d, 0x222, 0x00}, // ia32_perf_fixed_ctr_ctrl: ensure 3 FFCs enabled
        {MSR_WRITE, 0x38f, 0x0f, 0x07},  // ia32_perf_global_ctrl: enable 4 PMCs & 3 FFCs
        {MSR_STOP, 0x00, 0x00}};

    fd = loadDriver();
    unsigned int ngx_ncpu = 0;
    ngx_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    printf("ncpu = %d\n", ngx_ncpu);
    for (unsigned int i = 0; i < ngx_ncpu; i++)
    {
        struct MsrInOut msr_change_cpu_0[] = {
            {MSR_CHANGE_CPU, 0x0, i},
            {MSR_STOP, 0x00, 0x00}};
        ioctl(fd, IOCTL_MSR_CMDS, &msr_change_cpu_0[0]);
        ioctl(fd, IOCTL_MSR_CMDS, &msr_start[0]);
    }

    if (argc == 1)
        printf("%d\n", IOCTL_MSR_CMDS);
    closeDriver(fd);
    return 0;
}