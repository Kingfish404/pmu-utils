#include <linux/kernel.h>
#include <linux/module.h>

#if defined(__x86_64__)
#include <linux/smp.h>
#include <linux/version.h>
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef KERN_INFO
#define KERN_INFO "[KERN_INFO] "
#endif

typedef unsigned long long uint64_t;

/**
 * For x86_64
 * printc4 - Print CR4
 *
 * For arm64
 * printmrs - Print PMU registers
 */
static void print_msr(void)
{
    uint64_t output;
#if defined(__x86_64__) || defined(__amd64__) || defined(__i386__)
    // Read back CR4 to check the bit.
    __asm__("\t mov %%cr4,%0"
            : "=r"(output));
    printk(KERN_INFO "%llu", output);
#elif defined(__aarch64__) || defined(__arm__)
    asm volatile("mrs %0, pmintenclr_el1" : "=r"(output));
    printk(KERN_INFO "pmintenclr_el1:0x%llx", output);
    asm volatile("mrs %0, pmcntenset_el0" : "=r"(output));
    printk(KERN_INFO "pmcntenset_el0:0x%llx", output);
    asm volatile("mrs %0, pmuserenr_el0" : "=r"(output));
    printk(KERN_INFO "pmuserenr_el0:0x%llx", output);
    asm volatile("mrs %0, pmcr_el0" : "=r"(output));
    printk(KERN_INFO "pmcr_el0:0x%llx", output);
    asm volatile("mrs %0, pmccfiltr_el0" : "=r"(output));
    printk(KERN_INFO "pmccfiltr_el0:0x%llx", output);
#elif defined(__riscv)
    printk(KERN_INFO "Not implemented for riscv");
#else
    printk(KERN_INFO "Not implemented for unknown architecture");
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
    printk(KERN_INFO "Ran on Processor %d", smp_processor_id());
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

    printk(KERN_INFO "enable finished: cpu%d", smp_processor_id());
#elif defined(__riscv)
    printk(KERN_INFO "Not implemented for riscv");
#else
    printk(KERN_INFO "Not implemented for unknown architecture");
#endif
    print_msr();
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
    printk(KERN_INFO "Ran on Processor %d", smp_processor_id());
#elif defined(__aarch64__) || defined(__arm__)
    uint64_t val;
    /* Disable all counters */
    asm volatile("msr pmcntenset_el0, %0" ::"r"((uint64_t)0));
    /* Disable user-mode access to counters. */
    asm volatile("mrs %0, pmuserenr_el0" : "=r"(val));
    val &= ~(1);
    asm volatile("msr pmuserenr_el0, %0" : : "r"(val));
#elif defined(__riscv)
    printk(KERN_INFO "Not implemented for riscv");
#else
    printk(KERN_INFO "Not implemented for unknown architecture");
#endif
    print_msr();
}

int pmu_driver_init(void)
{
    on_each_cpu(enable_pmu, NULL, 1);
    printk("Enable pmu\n");
    return 0;
}

void pmu_driver_exit(void)
{
    on_each_cpu(disable_pmu, NULL, 1);
    printk("Disable pmu, bye, kernel!\n");
}

module_init(pmu_driver_init);
module_exit(pmu_driver_exit);

MODULE_LICENSE("GPL");
