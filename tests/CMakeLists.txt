cmake_minimum_required(VERSION 3.10)

project(test)

file(GLOB LIB_PMU ./../libpmu/*c)

add_executable(test_all all.c ${LIB_PMU})
add_executable(test_basic basic.c ${LIB_PMU})
add_executable(test_set_pmu set_pmu.c ${LIB_PMU})
add_executable(test_header header.c)
add_executable(test_header_basic header_basic.c)

add_custom_target(test
    DEPENDS test_all test_basic test_set_pmu 
    DEPENDS test_header test_header_basic
    COMMAND taskset -c 0 sudo ./test_header_basic
    COMMAND taskset -c 0 sudo ./test_basic
    COMMAND taskset -c 0 sudo ./test_header
    COMMAND taskset -c 0 sudo ./test_set_pmu
    COMMAND taskset -c 0 sudo ./test_all
)