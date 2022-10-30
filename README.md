# pmu-linux-embedded

## require

- linux source code
- disable Secure Boot in UEFI (BIOS) settings
    - `insmod usr-enable/enable_pmc.ko` need
- root privilege

## Use

```shell
# usage: compile and enable (notice: if have enabled, it need to disable first)
make clean && make
# usage: disable
make disable
# test
make test_asm
make test_pmu
```

如果要修改pmu读取的事件，修改`msr-driver`里`usr_enable.c`中的事件编码，并重新使能`pmu`，事件参考手册: https://perfmon-events.intel.com，部分参考[Skylake](./perfmon-events.md)

## Reference

- [mattferroni/msr-driver: A simple kernel module to read MSRs on Intel machines.](https://github.com/mattferroni/msr-driver)
