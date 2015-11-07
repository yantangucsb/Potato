#This is the makefile to make the emulator
CC=gcc
LD=gcc

CFLAGS=-O2 -std=gnu99 -g
OBJS= InodeAccess.o DiskEmulator.o FileSystem.o SuperBlock.o FreeListNode.o
FUSEFLAGS=`pkg-config fuse --cflags --libs`
SRCS=DiskEmulator.c

all: init test

init: $(OBJS) InitFSTest

test: $(OBJS) Layer0Test

InitFSTest: $(OBJS) InitFSTest.o
	$(CC) $(CFLAGS) -o $@ $^

Layer0Test: $(OBJS) Layer0Test.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -fr *.o Layer0Test InitFSTest

