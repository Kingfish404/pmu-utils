CC :=gcc
CPP :=g++
CFLAGS :=
CPPFLAGS :=
LIBC = $(wildcard ./../libpmu/*.c)
C_SOURCES :=$(wildcard *.c)
C_EXECUTABLE :=$(C_SOURCES:.c=.c.out)
CPP_SOURCES :=$(wildcard *.cpp)
CPP_EXECUTABLE :=$(CPP_SOURCES:.cpp=.cpp.out)
 
all:$(C_EXECUTABLE) $(CPP_EXECUTABLE)

%.c.out: %.c
	$(CC) $(LIBC) $< -o $@ $(CFLAGS)

%.cpp.out: %.cpp
	$(CPP) $(LIBC) $< -o $@ $(CPPFLAGS)

asm_c:
	$(CC) -S $(CFLAGS) $(C_SOURCES)

asm_cpp:
	$(CPP) -S $(CPPFLAGS) $(CPP_SOURCES)

clean:
	rm -rf $(C_EXECUTABLE)
	rm -rf $(CPP_EXECUTABLE)
	rm -rf *.s
	rm -rf *.out

test:
	make clean 

	$(CC) $(LIBC) basic.c -o basic.c.out $(CFLAGS)
	$(CC) $(LIBC) all.c -o all.c.out $(CFLAGS)

	taskset -c 0 sudo ./basic.c.out
	taskset -c 0 sudo ./all.c.out
