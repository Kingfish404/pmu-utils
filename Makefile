
pmu_utils: module/pmu_utils.c
	cd module && make

all: pmu_utils

format:
	black scripts