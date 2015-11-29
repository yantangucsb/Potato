#This is the makefile to make the emulator
CC=gcc
LD=gcc

CFLAGS=-O2 -std=gnu99 -g
OBJS=SysCall.o OpenFileTable.o DataBlockAccess.o InodeAccess.o DiskEmulator.o FileSystem.o SuperBlock.o FreeListNode.o
FUSEFLAGS=`pkg-config fuse --cflags --libs`
SRCS=DiskEmulator.c

all: mkfs init test fuse

mkfs: $(OBJS) mkfs

init: $(OBJS) InitFSTest

test: $(OBJS) Layer0Test Layer1Test SyscallTest

fuse: $(OBJS) fuse_potato

mksf: $(OBJS) mkfs.o
	$(CC) $(CFLAGS) -o $@ $^

SyscallTest: $(OBJS) SyscallTest.o
	$(CC) $(CFLAGS) -o $@ $^
	
Layer1Test: $(OBJS) Layer1Test.o
	$(CC) $(CFLAGS) -o $@ $^

InitFSTest: $(OBJS) InitFSTest.o
	$(CC) $(CFLAGS) -o $@ $^

Layer0Test: $(OBJS) Layer0Test.o
	$(CC) $(CFLAGS) -o $@ $^

fuse_potato: $(OBJS) fuse_potato.c
	$(CC) $(CFLAGS) fuse_potato.c $(FUSEFLAGS) -o $@ $(OBJS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -fr *.o Layer0Test InitFSTest InitFSTest mkfs fuse_potato

