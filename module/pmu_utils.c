#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>

#if defined(__x86_64__)
#include <linux/smp.h>
#include <linux/version.h>
#endif

#if defined(__riscv)
#include <asm/sbi.h>
#endif

#define PMU_DEVICE_NAME "pmu_utils"
#define PMU_DEVICE_PATH "/dev/" PMU_DEVICE_NAME

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned long long uint64_t;

/**
 * For x86_64
 * printc4 - Print CR4
 *
 * For arm64
 * printmrs - Print PMU registers
 */
static void print_msr(void *info)
{
    uint64_t output = 0;
    int num_sets = 0;
    int associativity = 0;
    int line_size = 0;
#if defined(__x86_64__) || defined(__amd64__) || defined(__i386__)
    // Read back CR4 to check the bit.
    __asm__("\t mov %%cr4,%0"
            : "=r"(output));
    pr_info("%llu", output);
#elif defined(__aarch64__) || defined(__arm__)
    asm volatile("mrs %0, pmintenclr_el1" : "=r"(output));
    pr_info("pmintenclr_el1: 0x%llx", output);
    asm volatile("mrs %0, pmcntenset_el0" : "=r"(output));
    pr_info("pmcntenset_el0: 0x%llx", output);
    asm volatile("mrs %0, pmuserenr_el0" : "=r"(output));
    pr_info("pmuserenr_el0: 0x%llx", output);
    asm volatile("mrs %0, pmcr_el0" : "=r"(output));
    pr_info("pmcr_el0: 0x%llx", output);
    asm volatile("mrs %0, pmccfiltr_el0" : "=r"(output));
    pr_info("pmccfiltr_el0: 0x%llx", output);
    // https://developer.arm.com/documentation/ddi0601/2025-06/AArch64-Registers/CCSIDR-EL1--Current-Cache-Size-ID-Register
    asm volatile("mrs %0, CCSIDR_EL1" : "=r"(output));
    pr_info("CCSIDR_EL1: 0x%llx", output);
    num_sets = ((output >> 32) & 0xffffff) + 1;
    associativity = ((output >> 3) & 0x1fffff) + 1;
    line_size = (output & 0x7) + 4;
    pr_info("Cache Sets: %d, Associativity: %d, Line Size: %d\n",
            num_sets, associativity, 1 << line_size);
#elif defined(__riscv)
    pr_info("Not implemented for riscv");
#else
    pr_info("Not implemented for unknown architecture");
#endif
}

/**
 * For x86_64
 * setc4b8 - Set CR4, Bit 8 (9th bit from the right) to enable
 *
 * For arm64
 *
 */
static void enable_pmu(void *info)
{
#if defined(__x86_64__) || defined(__amd64__) || defined(__i386__)
    // Set CR4, Bit 8 (9th bit from the right)  to enable
    asm volatile(
        "push   %rax\n\t"
        "mov    %cr4,%rax\n\t"
        "or     $(1 << 8), %rax\n\t"
        "mov    %rax, %cr4\n\t"
        "wbinvd\n\t"
        "pop    %rax");
    // Check which CPU we are on:
#elif defined(__aarch64__) || defined(__arm__)
    uint64_t val;
    /* Disable counter overflow interrupt */
    /* 0-30 bit corresponds to different pmu counter */
    asm volatile("msr pmintenclr_el1, %0" : : "r"((uint64_t)(1 << 31) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0)));

    /* Enable cycle counter */
    /* 0-30 bit corresponds to different pmu counter */
    asm volatile("msr pmcntenset_el0, %0" : : "r"(BIT(31) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0)));

    /* Enable user-mode access to pmu counters. */
    asm volatile("msr pmuserenr_el0, %0" : : "r"(BIT(0) | BIT(2) | BIT(3)));

    /* Clear counters and start */
    asm volatile("mrs %0, pmcr_el0" : "=r"(val));
    val |= (BIT(0) | BIT(1) | BIT(2) | BIT(6));

    isb();
    asm volatile("msr pmcr_el0, %0" : : "r"(val));

    val = BIT(27);
    asm volatile("msr pmccfiltr_el0, %0" : : "r"(val));
#elif defined(__riscv)
    struct sbiret r;
    r = sbi_ecall(SBI_EXT_PMU, SBI_EXT_PMU_COUNTER_START, 0x0,
                  0x7, SBI_PMU_START_FLAG_SET_INIT_VALUE, 0, 0, 0);
    asm volatile("csrsi scounteren, 0x7");
#else
    pr_info("Not implemented for unknown architecture");
#endif
}

/**
 * clearc4b8 - Clear CR4, Bit 8 (9th bit from the right) to disable
 */
static void disable_pmu(void *info)
{
#if defined(__x86_64__) || defined(__amd64__) || defined(__i386__)
    asm volatile(
        "push   %rax\n\t"
        "push   %rbx\n\t"
        "mov    %cr4, %rax\n\t"
        "mov    $(1 << 8), %rbx\n\t"
        "not    %rbx\n\t"
        "and    %rbx, %rax\n\t"
        "mov    %rax, %cr4\n\t"
        "wbinvd\n\t"
        "pop    %rbx\n\t"
        "pop    %rax\n\t");
    pr_info("Ran on Processor %d", smp_processor_id());
#elif defined(__aarch64__) || defined(__arm__)
    uint64_t val;
    /* Disable all counters */
    asm volatile("msr pmcntenset_el0, %0" ::"r"((uint64_t)0));
    /* Disable user-mode access to counters. */
    asm volatile("mrs %0, pmuserenr_el0" : "=r"(val));
    val &= ~(1);
    asm volatile("msr pmuserenr_el0, %0" : : "r"(val));
#elif defined(__riscv)
    struct sbiret r;
    r = sbi_ecall(SBI_EXT_PMU, SBI_EXT_PMU_COUNTER_START, 0x0,
                  0x0, SBI_PMU_START_FLAG_SET_INIT_VALUE, 0, 0, 0);
    asm volatile("csrsi scounteren, 0x7");
#else
    pr_info("Not implemented for unknown architecture");
#endif
}

static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
    switch (ioctl_num)
    {
    default:
        enable_pmu(NULL);
        return 0;
    }
}

static int device_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    return 0;
}

static struct file_operations f_ops = {.owner = THIS_MODULE,
                                       .unlocked_ioctl = device_ioctl,
                                       .open = device_open,
                                       .release = device_release};

static struct miscdevice misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = PMU_DEVICE_NAME,
    .fops = &f_ops,
    .mode = S_IRWXUGO,
};

int pmu_driver_init(void);
void pmu_driver_exit(void);

int pmu_driver_init(void)
{
    int r = 0;
    on_each_cpu(enable_pmu, NULL, 1);
    on_each_cpu(print_msr, NULL, 1);

    r = misc_register(&misc_dev);
    if (r != 0)
    {
        pr_alert("Failed registering device with %d\n", r);
        return -ENXIO;
    }

    pr_info("Loaded pmu_utils.\n");
    return 0;
}

void pmu_driver_exit(void)
{
    on_each_cpu(disable_pmu, NULL, 1);
    misc_deregister(&misc_dev);

    pr_info("Removed pmu_utils, bye~, kernel.\n");
}

module_init(pmu_driver_init);
module_exit(pmu_driver_exit);

MODULE_LICENSE("GPL");
