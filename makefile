CC=gcc
CFLAGS=-Wall
CFLAGS+=`pkg-config --cflags x11`
LDFLAGS=`pkg-config --libs x11`

all: x-event-server

x-event-server: hooks.o

install: x-event-server
	install x-event-server /usr/local/bin/x-event-server

clean:
	rm -f x-event-server *.o

run: x-event-server
	./x-event-server
