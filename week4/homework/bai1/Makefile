# Makefile for String Processing TCP Server/Client
# IT4062E - Practical Exercise 04

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
	@echo "Compiling server..."
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_SRC) $(LIBS)
	@echo "Server compiled successfully: $(SERVER_TARGET)"

# Build client
$(CLIENT_TARGET): $(CLIENT_SRC)
	@echo "Compiling client..."
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SRC) $(LIBS)
	@echo "Client compiled successfully: $(CLIENT_TARGET)"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
ifeq ($(OS),Windows_NT)
	@if exist $(SERVER_TARGET) del $(SERVER_TARGET)
	@if exist $(CLIENT_TARGET) del $(CLIENT_TARGET)
else
	@rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
endif
	@echo "Clean completed"

# Run server (example with port 5500)
run-server: $(SERVER_TARGET)
	@echo "Starting server on port 5500..."
	@echo "Usage: ./$(SERVER_TARGET) <port_number>"
	@echo "Example: ./$(SERVER_TARGET) 5500"

# Run client (example with localhost:5500)
run-client: $(CLIENT_TARGET)
	@echo "Starting client..."
	@echo "Usage: ./$(CLIENT_TARGET) <server_ip> <port_number>"
	@echo "Example: ./$(CLIENT_TARGET) 127.0.0.1 5500"

# Test the application
test: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Testing String Processing Application"
	@echo "1. Start server: ./$(SERVER_TARGET) 5500"
	@echo "2. Start client: ./$(CLIENT_TARGET) 127.0.0.1 5500"
	@echo "3. Test with strings like: 1ab23c, 123abc#, abc123"

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build both server and client"
	@echo "  server       - Build server only"
	@echo "  client       - Build client only"
	@echo "  clean        - Remove build artifacts"
	@echo "  run-server   - Show server usage"
	@echo "  run-client   - Show client usage"
	@echo "  test         - Show testing instructions"
	@echo "  help         - Show this help"

# Phony targets
.PHONY: all clean run-server run-client test help
