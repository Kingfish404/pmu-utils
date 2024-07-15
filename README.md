# PMU Utils

## Requirements

Currently, only Linux is supported. The library does not rely on any other library. It uses only standard C functionality.

## Features
- Support for x86_64 chips (Intel `ia64` and AMD `amd64`) and ARM (`aarch64`) chips.
- Support for reading and writing to the MSR register of the Performance Monitoring Unit (PMU).
- Support for reading and writing the Performance Monitoring Counter (PMC) value.
- Support for reading the Performance Monitoring Event (PME).
   - For x86_64 `ia64` chips, the PME is getting from the [intel/perfmon](https://github.com/intel/perfmon)
   - For x86_64 `amd64` and `aarch64` chips, the PME is getting from the Linux Perf tool.

Read the following sections for more details.

## Linux Usage

Preparation:

```shell
# Building the kernel module requires the kernel headers of the kernel. On Ubuntu or Debian, they can be installed by running
sudo apt install linux-headers-$(uname -r)

# The kernel module can be built by running
make

# The resulting kernel module can be loaded using
sudo insmod module/pmu_utils.ko

# Additionally step for x86_64 chips machines
sudo modprobe msr   # load msr module

# build the PMU Event Header for x86-64 chips (for `ia64` and `amd64` architectures), or for ARM chips
cd scripts
python3 pmu_intel_utils.py # for x86-64 ia64 chips
python3 pmu_intel_utils.py --hybridcore # if chips have hybrid core
python3 pmu_amd_utils.py   # for x86-64 amd64 chips
python3 pmu_arm_utils.py   # for ARM chips
# then you could see the PMU header: `ia64_pmu_event.h`, `amd64_pmu_event.h`, or `arm_pmu_event.h`
# just include the header in your c or c++ project
```

Using the library:  
Refer to the `tests` directory for more details about how to use the library. All the functions are defined in `libpmu/pmu.h`.

```c
#include <pmu_utils.h>
```

### For x86_64 chips

```c
uint64_t start_pmc = 0, end_pmc = 0;
uint64_t hexcode = 0x410000, core = 0, pmc_id = 0;
int msr_fd = pmu_open_msr(core); // open msr
pmu_set_msr_event(msr_fd, hexcode, core); // set event
pmu_record_start(msr_fd); // start record
pmu_set_pmc(msr_fd, pmc_id, 0); // set 0 for PMC[pmc_id]

start_pmc = pmu_get_rdpmc(pmc_id); // read PMC[pmc_id]
/*
 code that you want to measure
*/
end_pmc = pmu_get_rdpmc(pmc_id); // read PMC[pmc_id]
```

### For aarch64 chips

```c
uint64_t pmc_id0 = 0, pmc_id1 = 1;
pmu_set_event(0x0, NULL, evet_code[0], pmc_id0);
pmu_set_event(0x0, NULL, evet_code[0], pmc_id1);
memset(mem, 0, 4096);

pmu_values[0] = pmu_get_rdpmc(0);
pmu_values[1] = pmu_get_rdpmc(1);
/*
code that you want to measure
*/
pmu_values[0] = pmu_get_rdpmc(0) - pmu_values[0];
pmu_values[1] = pmu_get_rdpmc(1) - pmu_values[1];
``````

## Test and Example

Seen in `tests` directory.

```shell
cd tests && mkdir -p build && cd build && cmake .. && make tests
```

## Script Utils

Change the directory to `scripts` and run `*utils.py` for your architecture.

## References
- [Intel® 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [AMD64 Architecture Programmer's Manual, Volumes 1-5 | AMD](https://www.amd.com/en/support/tech-docs/amd64-architecture-programmers-manual-volumes-1-5)
- [Arm A-profile A64 Instruction Set Architecture](https://developer.arm.com/documentation/ddi0602/latest)
- [Specifications – RISC-V International](https://riscv.org/technical/specifications/)
- [misc0110/PTEditor: A small library to modify all page-table levels of all processes from user space for x86_64 and ARMv8.](https://github.com/misc0110/PTEditor)
- [mattferroni/msr-driver: A simple kernel module to read MSRs on Intel machines.](https://github.com/mattferroni/msr-driver)
- [IAIK/transientfail: Website and PoC collection for transient execution attacks](https://github.com/IAIK/transientfail)
- [andikleen/pmu-tools: Intel PMU profiling tools](https://github.com/andikleen/pmu-tools)
- [dadaKh/pmu-reader: PMU-reader for Linux](https://github.com/dadaKh/pmu-reader)
