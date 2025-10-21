# Makefile for File Transfer TCP Server/Client
# IT4062E - Practical Exercise 04 - Exercise 2

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Windows specific settings
ifeq ($(OS),Windows_NT)
    LIBS = -lws2_32
    SERVER_TARGET = server.exe
    CLIENT_TARGET = client.exe
else
    LIBS = 
    SERVER_TARGET = server
    CLIENT_TARGET = client
endif

# Source files
SERVER_SRC = server.c
CLIENT_SRC = client.c

# Default target
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Build server
$(SERVER_TARGET): $(SERVER_SRC)
	@echo "Compiling file transfer server..."
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_SRC) $(LIBS)
	@echo "Server compiled successfully: $(SERVER_TARGET)"

# Build client
$(CLIENT_TARGET): $(CLIENT_SRC)
	@echo "Compiling file transfer client..."
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SRC) $(LIBS)
	@echo "Client compiled successfully: $(CLIENT_TARGET)"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
ifeq ($(OS),Windows_NT)
	@if exist $(SERVER_TARGET) del $(SERVER_TARGET)
	@if exist $(CLIENT_TARGET) del $(CLIENT_TARGET)
	@if exist uploads rmdir /s /q uploads
else
	@rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
	@rm -rf uploads
endif
	@echo "Clean completed"

# Create test files
test-files:
	@echo "Creating test files..."
	@mkdir -p test_files
	@echo "This is a test file for file transfer." > test_files/test.txt
	@echo "Line 2: Testing file transfer functionality." >> test_files/test.txt
	@echo "Line 3: This file contains multiple lines." >> test_files/test.txt
	@echo "Line 4: Testing binary data transfer." >> test_files/test.txt
	@echo "Test file created: test_files/test.txt"
	@echo "Test files created successfully"

# Run server (example with port 5500)
run-server: $(SERVER_TARGET)
	@echo "Starting file transfer server on port 5500..."
	@echo "Usage: ./$(SERVER_TARGET) <port_number>"
	@echo "Example: ./$(SERVER_TARGET) 5500"
	@echo "Upload directory will be created automatically"

# Run client (example with localhost:5500)
run-client: $(CLIENT_TARGET)
	@echo "Starting file transfer client..."
	@echo "Usage: ./$(CLIENT_TARGET) <server_ip> <port_number>"
	@echo "Example: ./$(CLIENT_TARGET) 127.0.0.1 5500"

# Test the application
test: $(SERVER_TARGET) $(CLIENT_TARGET) test-files
	@echo "Testing File Transfer Application"
	@echo "1. Start server: ./$(SERVER_TARGET) 5500"
	@echo "2. Start client: ./$(CLIENT_TARGET) 127.0.0.1 5500"
	@echo "3. Test with files:"
	@echo "   - test_files/test.txt"
	@echo "   - Any file up to 100MB"
	@echo "4. Check uploads/ directory for received files"

# Create upload directory
setup: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Setting up file transfer application..."
	@mkdir -p uploads
	@echo "Upload directory created: uploads/"
	@echo "Setup completed"

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build both server and client"
	@echo "  server       - Build server only"
	@echo "  client       - Build client only"
	@echo "  clean        - Remove build artifacts and uploads"
	@echo "  test-files   - Create test files for testing"
	@echo "  setup        - Create upload directory"
	@echo "  run-server   - Show server usage"
	@echo "  run-client   - Show client usage"
	@echo "  test         - Show testing instructions"
	@echo "  help         - Show this help"

# Phony targets
.PHONY: all clean test-files run-server run-client test setup help
