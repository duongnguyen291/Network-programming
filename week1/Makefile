CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -O2

all: exercise1 exercise2

exercise1: exercise1.c
	$(CC) $(CFLAGS) $< -o $@

exercise2: exercise2.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f exercise1 exercise2 *.o
