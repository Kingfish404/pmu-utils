# PMU Utils

## Requirements

Currently, only Linux is supported. The library does not rely on any other library. It uses only standard C functionality.

## Usage

Install `pmu-enable-driver`, set CR4.bit8 to 1 to enable `RDPMC` instruction.

```shell
cd pmu-enable-driver && make && make inmod
```

Refer to `tests` directory for more details about how to use the library. All the functions are defined in `libpmu/pmu.h`.

## Test and Example

Seen in `tests` directory.

```shell
cd tests && make test
```

## References or Related
- [misc0110/PTEditor: A small library to modify all page-table levels of all processes from user space for x86_64 and ARMv8.](https://github.com/misc0110/PTEditor)
- [mattferroni/msr-driver: A simple kernel module to read MSRs on Intel machines.](https://github.com/mattferroni/msr-driver)
- [IAIK/transientfail: Website and PoC collection for transient execution attacks](https://github.com/IAIK/transientfail)
- [Intel® 64 and IA-32 Architectures Software Developer’s Manual Volume 2D: Instruction Set Reference](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2d-manual.pdf)
- [PerfMon Events](https://perfmon-events.intel.com/#)
- [dadaKh/pmu-reader: PMU-reader for Linux](https://github.com/dadaKh/pmu-reader)
