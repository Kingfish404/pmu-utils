obj-m := pmu_enable.o

all :
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

insmod:
	insmod ./pmu_enable.ko

rmmod:
	rmmod pmu_enable

kern_log:
	tail /var/log/kern.log

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
