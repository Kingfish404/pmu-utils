# user config
target=1	# choise cpu id

# pmu command
enable: msr-driver/kernel/msrdrv_util.mod
	cd ./msr-driver/ && make usr
	cd ./msr-driver/pmc-enable-cr4/ && sudo make
	sudo taskset -c $(target) insmod ./msr-driver/pmc-enable-cr4/enable_pmc.ko
	sudo bash msr-driver/pmc_install.sh

disable:
	sudo rmmod enable_pmc
	sudo bash msr-driver/pmc_uninstall.sh 

msr-driver/kernel/msrdrv_util.mod:
	cd msr-driver/kernel/ && make

clean:
	cd ./msr-driver/ && make clean
	cd ./msr-driver/kernel/ && make clean
	cd ./msr-driver/pmc-enable-cr4/ && sudo make clean

# test command
test_asm:
	cd test && 	gcc -o test.out test_pmu.c
	cd test && ./test.out

test_pmu:
	cd test && 	gcc -o test.out test_pmu.c
	cd test && \
	n=10;\
		while [ $${n} -gt 1 ] ; do\
		taskset -c $(target) ./test.out;\
		n=`expr $$n - 1`;\
	done;

test_zombieload:
	cd test && 	gcc -o test.out test_zombie1.c
	cd test && \
	n=10;\
		while [ $${n} -gt 1 ] ; do\
		sudo taskset -c $(target) ./test.out;\
		n=`expr $$n - 1`;\
	done;
