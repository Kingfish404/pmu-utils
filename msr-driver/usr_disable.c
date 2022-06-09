#include "include/msrdrv.h"
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
int main(void)
{
    int fd;

    struct MsrInOut msr_change_cpu_1[] = {
        {MSR_CHANGE_CPU, 0x38f, 0x00, 0x00},
        {MSR_STOP, 0x00, 0x00, 1}};

    struct MsrInOut msr_stop[] = {
        {MSR_WRITE, 0x38f, 0x00, 0x00}, // ia32_perf_global_ctrl: disable 4 PMCs & 3 FFCs
        {MSR_WRITE, 0x38d, 0x00, 0x00}, // ia32_perf_fixed_ctr_ctrl: clean up FFC ctrls
        {MSR_READ, 0xc1, 0x00},         // ia32_pmc0: read value (35-5)
        {MSR_READ, 0xc2, 0x00},         // ia32_pmc1: read value (35-5)
        {MSR_READ, 0xc3, 0x00},         // ia32_pmc2: read value (35-5)
        {MSR_READ, 0xc4, 0x00},         // ia32_pmc3: read value (35-5)
        {MSR_READ, 0x309, 0x00},        // ia32_fixed_ctr0: read value (35-17)
        {MSR_READ, 0x30a, 0x00},        // ia32_fixed_ctr1: read value (35-17)
        {MSR_READ, 0x30b, 0x00},        // ia32_fixed_ctr2: read value (35-17)
        {MSR_STOP, 0x00, 0x00}};

    fd = loadDriver();
    unsigned int ngx_ncpu = 0;
    ngx_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    // printf("ncpu = %d\n", ngx_ncpu);
    for (unsigned int i = 0; i < ngx_ncpu; i++)
    {
        struct MsrInOut msr_change_cpu[] = {
            {MSR_CHANGE_CPU, 0x38f, 0x0, 0x0},
            {MSR_STOP, 0x00, 0x00, i}};
        ioctl(fd, IOCTL_MSR_CMDS, &msr_change_cpu[0]);
        ioctl(fd, IOCTL_MSR_CMDS, &msr_stop[0]);
    }

    closeDriver(fd);
    return 0;
}