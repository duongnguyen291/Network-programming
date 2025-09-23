CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -O2

# Default target
all: exercise1 exercise2

# Compile targets
exercise1: exercise1.c
	$(CC) $(CFLAGS) $< -o $@

exercise2: exercise2.c
	$(CC) $(CFLAGS) $< -o $@

# Run targets
run1: exercise1
	./exercise1

run2: exercise2
	./exercise2

# Run both programs
run: run1 run2

# Clean compiled files
clean:
	rm -f exercise1 exercise2 *.o *.exe

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Compile both programs (default)"
	@echo "  exercise1 - Compile exercise1 only"
	@echo "  exercise2 - Compile exercise2 only"
	@echo "  run1    - Compile and run exercise1"
	@echo "  run2    - Compile and run exercise2"
	@echo "  run     - Compile and run both programs"
	@echo "  clean   - Remove compiled files"
	@echo "  help    - Show this help message"

# Phony targets
.PHONY: all run1 run2 run clean help
