
pmu_utils: module/pmu_utils.c
	cd module && make

all: pmu_utils

load: pmu_utils
	sudo insmod module/pmu_utils.ko

unload:
	-sudo rmmod pmu_utils

clean:
	make -C module clean

test:
	make -C tests ctest

test_verbose:
	make -C tests ctest_verbose

format:
	python3 -m black scripts
