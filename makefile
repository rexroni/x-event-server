CC=gcc
CFLAGS=-Wall
CFLAGS+=`pkg-config --cflags x11`
LDFLAGS=`pkg-config --libs x11`

all: main

main: hooks.o

clean:
	rm -f main *.o

run: main
	./main
