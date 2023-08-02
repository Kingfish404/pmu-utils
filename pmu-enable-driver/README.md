# PMU Enable Driver for Linux

Set CR4.bit8 to 1 to enable `RDPMC` instruction.

```shell
make all        # compile
make insmod     # install module
make rmmod      # remove module
make kern_log   # show kernel log
```
