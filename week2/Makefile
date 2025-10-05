# Makefile for resolver program

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lws2_32
TARGET = resolver
SOURCE = resolver.c

# Thêm target cho bài 2
TARGET2 = resolver2
SOURCE2 = resolver2.c

# Phần mở rộng executable cho Windows
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    RM = del /f
    RM_FLAGS = 2>nul || true
else
    RM = rm -f
    RM_FLAGS = 
endif

# Rule mặc định
all: $(TARGET)

# Build resolver2
$(TARGET2): $(SOURCE2)
	$(CC) $(CFLAGS) -o $(TARGET2)$(EXE_EXT) $(SOURCE2) $(LDFLAGS)

# Rule để build target từ source
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

# Rule để clean các file được tạo
clean:
	$(RM) $(TARGET) $(RM_FLAGS)

# Rule để rebuild (clean + build)
rebuild: clean all

# Test nhanh resolver1
test: $(TARGET)
	@echo "Testing resolver (exercise 1)"
	@./$(TARGET) google.com || ./$(TARGET).exe google.com

# Test batch resolver2 với file test nếu có
test2: $(TARGET2)
	@echo "Running batch tests for resolver2"
	@./$(TARGET2) resolver2_tests.txt || ./$(TARGET2).exe resolver2_tests.txt


# Rule để test chương trình
test: $(TARGET)
	@echo "Testing resolver program..."
	@echo "Test 1: Domain name"
	@.\$(TARGET) google.com
	@echo ""
	@echo "Test 2: Valid IP address"
	@.\$(TARGET) 8.8.8.8
	@echo ""
	@echo "Test 3: Invalid IP address"
	@.\$(TARGET) 1.2.3
	@echo ""
	@echo "Test 4: Out of range IP"
	@.\$(TARGET) 300.300.300.300

# Đánh dấu các rule không phải là file
.PHONY: all clean rebuild test