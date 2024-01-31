# PMU Utils

## Requirements

Currently, only Linux is supported. The library does not rely on any other library. It uses only standard C functionality.

## Linux Usage

Preparation:

```shell
# Building the kernel module requires the kernel headers of the kernel. On Ubuntu or Debian, they can be installed by running
sudo apt install linux-headers-$(uname -r)

# Both the library and the the kernel module can be build by running
make

# The resulting kernel module can be loaded using
sudo insmod module/pmu_utils.ko

# Additionally step for x86_64 chips machines
sudo modprobe msr   # load msr module
```

Using the library:  
Refer to `tests` directory for more details about how to use the library. All the functions are defined in `libpmu/pmu.h`.

```c
#include <pmu_utils.h>
```

### For x86_64 chips

```c
uint64_t start_pmc = 0, end_pmc = 0;
uint64_t hexcode = 0x410000, core = 0, pmc_id = 0;
int msr_fd = pmu_open_msr(core);             // open msr
pmu_set_msr_event(msr_fd, hexcode, core);    // set event
pmu_record_start(msr_fd);                    // start record
pmu_set_pmc(msr_fd, pmc_id, 0);              // set 0 for PMC[pmc_id]

start_pmc = pmu_get_rdpmc(pmc_id);        // read PMC[pmc_id]
/*
   code that you want to measure
*/
end_pmc = pmu_get_rdpmc(pmc_id);          // read PMC[pmc_id]
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

## References or Related
- [Intel® 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [AMD64 Architecture Programmer's Manual, Volumes 1-5 | AMD](https://www.amd.com/en/support/tech-docs/amd64-architecture-programmers-manual-volumes-1-5)
- [misc0110/PTEditor: A small library to modify all page-table levels of all processes from user space for x86_64 and ARMv8.](https://github.com/misc0110/PTEditor)
- [mattferroni/msr-driver: A simple kernel module to read MSRs on Intel machines.](https://github.com/mattferroni/msr-driver)
- [IAIK/transientfail: Website and PoC collection for transient execution attacks](https://github.com/IAIK/transientfail)
- [PerfMon Events](https://perfmon-events.intel.com/#)
- [dadaKh/pmu-reader: PMU-reader for Linux](https://github.com/dadaKh/pmu-reader)
