#This is the makefile to make the emulator
CC=gcc
LD=gcc

CFLAGS=-O2 -std=gnu99 -g
OBJS=DiskEmulator.o
FUSEFLAGS=`pkg-config fuse --cflags --libs`
SRCS=DiskEmulator.c

all: test

test: $(OBJS) Layer0Test

Layer0Test: $(OBJS) Layer0Test.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -fr *.o Layer0Test 

