#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static void printc4(void)
{
    typedef long unsigned int uint64_t;
    uint64_t output;
    // Read back CR4 to check the bit.
    __asm__("\t mov %%cr4,%0"
            : "=r"(output));
    printk(KERN_INFO "%lu", output);
}

static void setc4b8(void *info)
{
    // Set CR4, Bit 8 (9th bit from the right)  to enable
    asm volatile(
        "push   %rax\n\t"
        "mov    %cr4,%rax;\n\t"
        "or     $(1 << 8), %rax;\n\t"
        "mov    %rax, %cr4;\n\t"
        "wbinvd\n\t"
        "pop    %rax");
    // Check which CPU we are on:
    printk(KERN_INFO "Ran on Processor %d", smp_processor_id());
    printc4();
}

static void clearc4b8(void *info)
{
    printc4();
    asm volatile(
        "push   %rax\n\t"
        "push   %rbx\n\t"
        "mov    %cr4, %rax;\n\t"
        "mov    $(1 << 8), %rbx\n\t"
        "not    %rbx\n\t"
        "and    %rbx, %rax;\n\t"
        "mov    %rax, %cr4;\n\t"
        "wbinvd\n\t"
        "pop    %rbx\n\t"
        "pop    %rax\n\t");
    printk(KERN_INFO "Ran on Processor %d", smp_processor_id());
}

int pmu_driver_init(void)
{
    on_each_cpu(setc4b8, NULL, 0);
    return 0;
}

void pmu_driver_exit(void)
{
    on_each_cpu(clearc4b8, NULL, 0);
    printk("Bye, kernel!\n");
}

module_init(pmu_driver_init);
module_exit(pmu_driver_exit);
