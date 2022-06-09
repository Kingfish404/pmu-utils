# pmu-linux-embedded

## require

- linux source code
- disable Secure Boot in UEFI (BIOS) settings
    - `insmod usr-enable/enable_pmc.ko` need
- root privilege

## Use

```
make enable
```

如果要修改pmu读取的事件，修改msr-driver里usr_enable.c中的事件编码，并重新使能pmu，事件参考手册: https://perfmon-events.intel.com，部分参考[Skylake](./perfmon-events.md)

## Reference

- [mattferroni/msr-driver: A simple kernel module to read MSRs on Intel machines.](https://github.com/mattferroni/msr-driver)
