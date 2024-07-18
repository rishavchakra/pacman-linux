PWD:=$(CURDIR)
# CC=aarch64-linux-gnu-gcc
CFLAGS=-static
LDLIBS=-lm

SRCS = src/main.c src/cache.c
HEADERS = src/cache.h

OBJS = $(SRCS:src/*.c=.o)

TEST_SRCS = src/tests/timer.c

default: pacman
	@echo Compiled executable: pacman - run with qemu-aarch64 pacman

pacman: $(OBJS)
	$(CC) $(CFLAGS) $(SRCS) $(HEADERS) -o pacman $(LDLIBS)

test: test-timer test-kmod

test-timer: src/tests/timers.o
	$(CC) $(CFLAGS) src/tests/timers.c -o test_timer
	@echo Compiled test: test_timer - run with qemu-aarch64 test_timer

test-kmod: src/tests/kmodule.o src/kmodule.o
	$(CC) $(CFLAGS) src/tests/kmodule.c src/kmodule.c -o test_kmod
	@echo Compiled test: test_kmod - run with test_kmod

obj-m += kmodule/main.o

kmod:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm pacman
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
