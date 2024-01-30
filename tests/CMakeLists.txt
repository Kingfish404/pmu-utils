cmake_minimum_required(VERSION 3.10)

project(test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
file(GLOB LIB_PMU ./../libpmu/*c)

message( STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}" )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../header)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libpmu)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/x86_64)
    add_executable(test_all ${SRC_DIR}/all.c ${LIB_PMU})
    add_executable(test_basic ${SRC_DIR}/basic.c ${LIB_PMU})
    add_executable(test_set_pmu ${SRC_DIR}/set_pmu.c ${LIB_PMU})
    add_executable(test_header ${SRC_DIR}/header.c)
    add_executable(test_header_basic ${SRC_DIR}/header_basic.c)
    add_custom_target(tests
        DEPENDS test_all test_basic test_set_pmu 
        DEPENDS test_header test_header_basic
        COMMAND taskset -c 0 sudo ./test_header_basic
        COMMAND taskset -c 0 sudo ./test_basic
        COMMAND taskset -c 0 sudo ./test_header
        COMMAND taskset -c 0 sudo ./test_set_pmu
        COMMAND taskset -c 0 sudo ./test_all
    )
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/aarch64)
    add_executable(test_header ${SRC_DIR}/header.c)
    add_custom_target(tests
        DEPENDS test_header
        COMMAND taskset -c 0 sudo ./test_header
    )
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
    message("RISC-V is not supported yet")
else()
endif()