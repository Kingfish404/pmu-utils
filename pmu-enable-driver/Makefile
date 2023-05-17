obj-m := pmu_enable.o

all :
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

insmod:
	sudo insmod ./pmu_enable.ko

rmmod:
	sudo rmmod pmu_enable

kern_log:
	sudo tail /var/log/kern.log

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
