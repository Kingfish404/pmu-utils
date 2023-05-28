CC :=gcc
CPP :=g++
CFLAGS :=
CPPFLAGS :=
LIBC = $(wildcard ./../libpmu/*.c)
C_SOURCES :=$(wildcard *.c)
C_EXECUTABLE :=$(C_SOURCES:.c=.c.out)
CPP_SOURCES :=$(wildcard *.cpp)
CPP_EXECUTABLE :=$(CPP_SOURCES:.cpp=.cpp.out)
 
clean:
	rm -rf $(C_EXECUTABLE)
	rm -rf $(CPP_EXECUTABLE)
	rm -rf *.s
	rm -rf *.out

build:
	make clean 

	$(CC) $(LIBC) basic.c -o basic.c.out $(CFLAGS)
	$(CC) $(LIBC) all.c -o all.c.out $(CFLAGS)
	$(CC) $(LIBC) set_pmu.c -o set_pmu.c.out $(CFLAGS)
	$(CC) header.c -o header.c.out $(CFLAGS)

test:build
	taskset -c 0 sudo ./basic.c.out
	taskset -c 0 sudo ./all.c.out
	taskset -c 0 sudo ./header.c.out
