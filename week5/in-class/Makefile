# Makefile for TCP Chat Application
# IT4062E - HW41 Implementation

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
	@echo "Compiling TCP Chat Server..."
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_SRC) $(LIBS)
	@echo "Server compiled successfully: $(SERVER_TARGET)"

# Build client
$(CLIENT_TARGET): $(CLIENT_SRC)
	@echo "Compiling TCP Chat Client..."
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SRC) $(LIBS)
	@echo "Client compiled successfully: $(CLIENT_TARGET)"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
ifeq ($(OS),Windows_NT)
	@if exist $(SERVER_TARGET) del $(SERVER_TARGET)
	@if exist $(CLIENT_TARGET) del $(CLIENT_TARGET)
	@if exist logs rmdir /s /q logs
else
	@rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
	@rm -rf logs
endif
	@echo "Clean completed"

# Run server (example with port 5500)
run-server: $(SERVER_TARGET)
	@echo "Starting TCP Chat Server on port 5500..."
	@echo "Usage: ./$(SERVER_TARGET) <port_number>"
	@echo "Example: ./$(SERVER_TARGET) 5500"
	@echo "Log directory will be created automatically"

# Run client (example with localhost:5500)
run-client: $(CLIENT_TARGET)
	@echo "Starting TCP Chat Client..."
	@echo "Usage: ./$(CLIENT_TARGET) <server_ip> <port_number>"
	@echo "Example: ./$(CLIENT_TARGET) 127.0.0.1 5500"

# Test the application
test: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Testing TCP Chat Application"
	@echo "1. Start server: ./$(SERVER_TARGET) 5500"
	@echo "2. Start client: ./$(CLIENT_TARGET) 127.0.0.1 5500"
	@echo "3. Test sequence:"
	@echo "   - Login with username"
	@echo "   - Send messages"
	@echo "   - Logout"
	@echo "4. Check logs/ directory for log files"

# Create log directory
setup: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Setting up TCP Chat Application..."
	@mkdir -p logs
	@echo "Log directory created: logs/"
	@echo "Setup completed"

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build both server and client"
	@echo "  server       - Build server only"
	@echo "  client       - Build client only"
	@echo "  clean        - Remove build artifacts and logs"
	@echo "  setup        - Create log directory"
	@echo "  run-server   - Show server usage"
	@echo "  run-client   - Show client usage"
	@echo "  test         - Show testing instructions"
	@echo "  help         - Show this help"

# Phony targets
.PHONY: all clean setup run-server run-client test help
