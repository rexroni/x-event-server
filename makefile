CC=gcc
CFLAGS=-Wall
CFLAGS+=`pkg-config --cflags x11`
LDFLAGS=`pkg-config --libs x11`

all: main run

run:
	./main
