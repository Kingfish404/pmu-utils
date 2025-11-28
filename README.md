# PMU Utils

## Features
- Support for AMD64 (`x86_64`, Intel `ia64` and AMD `amd64`), ARM64 (`aarch64`), and RISC-V (`riscv`).
- Support for reading and writing to the MSR register of the Performance Monitoring Unit (PMU).
- Support for reading and writing the Performance Monitoring Counter (PMC) value.
- Support enable user space access to high resolution timers.
  - `"MRS %0, PMCCNTR_EL0` for ARM64 chips.
  - `rdcycle` for RISC-V chips.
- Support for reading the Performance Monitoring Event (PME).
  - For `x86_64` `ia64` chips, the PME is getting from the [intel/perfmon](https://github.com/intel/perfmon)
  - For `x86_64` `amd64` and `aarch64` chips, the PME is getting from the Linux Perf tool.

Read the following sections for more details.

## Preparation on Linux

Currently, only Linux is supported. The library does not rely on any other library. It uses only standard C functionality.

```shell
# Building the kernel module requires the kernel headers of the kernel. On Ubuntu or Debian, they can be installed by running
# The kernel module can be built by running
sudo apt install linux-headers-$(uname -r)
make unload load

# Additionally step for x86_64 chips machines
sudo modprobe msr   # load msr module

# For RISC-V or ARM chips with linux kernel version >= 6.6, enable userspace access to perf
echo 1 | sudo tee /proc/sys/kernel/perf_user_access
# https://github.com/torvalds/linux/blob/master/drivers/perf/riscv_pmu_sbi.c
```

## Script Utils

Change the directory to `scripts` and run `*utils.py` for your architecture.

```shell
# build the PMU Event Header for x86-64 chips (for `ia64` and `amd64` architectures), or for ARM chips
cd scripts
python3 pmu_init.py
# or for intel chips with hybrid core
python3 pmu_init.py --hybridcore
# equal to `python3 pmu_intel_utils.py --hybridcore`
# then you could see the PMU header: `ia64_pmu_event.h`, `amd64_pmu_event.h`, or `arm_pmu_event.h`
# just include the header in your c or c++ project
```

Yet another way to get the PMU event header is to use `perf` tool.

```shell
sudo perf list -j > this-cpu-pmu-events.json
```

## Using the library:  
Refer to the `tests` directory for more details about how to use the library.

Copy the header file (`*.h`) in `header` to your project include path or add the path of `header` to `CFLAGS`, after that, include the `pmu_utils.h`.

```c
#include <pmu_utils.h>
```

### For AMD64 (x86_64) chips

See [tests/src/x86_64/basic.c](tests/src/x86_64/basic.c) for more details.

### For ARM64 (aarch64) chips

See [tests/src/aarch64/basic.c](tests/src/aarch64/basic.c) for more details.

### For RISC-V (riscv64) chips

See [tests/src/riscv64/basic.c](tests/src/riscv64/basic.c) for more details.

## Test and Example

Seen in `tests` directory.

```shell
make test test_verbose
```

## References
- [Intel® 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [AMD64 Architecture Programmer's Manual, Volumes 1-5 | AMD](https://www.amd.com/en/support/tech-docs/amd64-architecture-programmers-manual-volumes-1-5)
- [Arm A-profile A64 Instruction Set Architecture](https://developer.arm.com/documentation/ddi0602/latest)
- [RISC-V Ratified Specifications](https://riscv.org/specifications/ratified/)
- [misc0110/PTEditor: A small library to modify all page-table levels of all processes from user space for x86_64 and ARMv8.](https://github.com/misc0110/PTEditor)
- [mattferroni/msr-driver: A simple kernel module to read MSRs on Intel machines.](https://github.com/mattferroni/msr-driver)
- [IAIK/transientfail: Website and PoC collection for transient execution attacks](https://github.com/IAIK/transientfail)
- [andikleen/pmu-tools: Intel PMU profiling tools](https://github.com/andikleen/pmu-tools)
- [dadaKh/pmu-reader: PMU-reader for Linux](https://github.com/dadaKh/pmu-reader)
- [cpredef/predef: Pre-defined Compiler Macros wiki](https://github.com/cpredef/predef)
