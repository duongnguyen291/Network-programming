CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lws2_32
TARGETS = server.exe client.exe

all: $(TARGETS)

server.exe: server.c
	$(CC) $(CFLAGS) -o server.exe server.c $(LDFLAGS)

client.exe: client.c
	$(CC) $(CFLAGS) -o client.exe client.c $(LDFLAGS)

clean:
	rm -f $(TARGETS) *.obj

.PHONY: all clean
