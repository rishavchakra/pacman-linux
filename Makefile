CC=aarch64-linux-gnu-gcc
CFLAGS=-static

SRCS = src/main.c
HEADERS = 

OBJS = $(SRCS:src/*.c=.o)

TEST_SRCS = src/tests/timer.c

default: pacman
	@echo Compiled executable: pacman - run with qemu-aarch64 pacman

pacman: $(OBJS)
	$(CC) $(CFLAGS) $(SRCS) $(HEADERS) -o pacman

test: test-timer

test-timer: src/tests/timers.o
	$(CC) $(CFLAGS) src/tests/timers.c -o test_timer
	@echo Compiled test: test_timer - run with qemu-aarch64 test_timer

clean:
	rm pacman
