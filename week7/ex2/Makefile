CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS =

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

.PHONY: run_server run_client
run_server: server
	./server 5500

run_client: client
	./client 127.0.0.1 5500
