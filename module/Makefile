obj-m := pmu_utils.o

all :
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

insmod:
	insmod ./pmu_utils.ko

rmmod:
	rmmod pmu_utils

kern_log:
	tail /var/log/kern.log

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
