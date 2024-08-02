PWD:=$(CURDIR)
CC=aarch64-linux-gnu-gcc
KMODCC=aarch64-linux-gnu-gcc-13
# Neoverse V1 of AWS instance uses armv8.4-a
CFLAGS=-static -march=armv8.4-a
KMODCFLAGS=-march=armv8.4-a
LDLIBS=-lm

SRCS = src/main.c src/cache.c src/memory.c src/kmodule.c src/pacman.c src/eviction_set.c src/pac.c
HEADERS = src/cache.h

OBJS = $(SRCS:src/*.c=.o)

TEST_SRCS = src/tests/timer.c src/tests/pac.c src/kmodule.c

default: pacman
	@echo Compiled executable: pacman - run with qemu-aarch64 pacman

pacman: $(OBJS)
	$(CC) $(CFLAGS) $(SRCS) $(HEADERS) -o pacman $(LDLIBS)

################################
# Tests
################################

test: test-timer test-kmod test-pac

test-timer: src/tests/timers.o
	$(CC) $(CFLAGS) src/tests/timers.c -o test_timer
	@echo Compiled test: test_timer - run with qemu-aarch64 test_timer

test-kmod: src/tests/kmodule.o src/kmodule.o
	$(CC) $(CFLAGS) src/tests/kmodule.c src/kmodule.c -o test_kmod
	@echo Compiled test: test_kmod - run with test_kmod

test-pac: src/tests/pac.o
	$(CC) $(CFLAGS) src/tests/pac.c -o test_pac
	@echo Compiled test: test_pac - run with qemu-aarch64 test_pac


################################
# Building the kernel module
################################

# Sources list
# the build system checks for this variable
obj-m += kmodule/main.o kmodule/pac_gadget.o

# Attach CFlags to kmod objects
CFLAGS_kmodule/main.o := -march=armv8.4-a
CFLAGS_kmodule/pac_gadget.o := -march=armv8.4-a

kmod:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) CC=$(KMODCC) modules
	@echo Compiled PACMAN kernel module
	@echo Start with sudo insmod kmodule/main.ko
	@echo Stop with sudo rmmod kmodule/main.ko

clean:
	rm pacman
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
