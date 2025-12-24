CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
LDFLAGS = -lm

TARGETS = server client
SOURCES = server.c client.c
OBJECTS = server.o client.o

.PHONY: all clean

all: $(TARGETS)

server: server.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

client: client.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGETS)
	rm -rf shared_files temp_files
	rm -f received_file.txt

.PHONY: run_server run_client
run_server: server
	./server 5500

run_client: client
	./client 127.0.0.1 5500
