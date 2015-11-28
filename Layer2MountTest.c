/**
 * Tests Layer 2 directories
 * by Jon
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "Directories.h"

void testBlockify();

int main(int args, char* argv[])
{
    #ifdef DEBUG

    if (args < 3) {
        printf("Usage: Layer2MountTest FS_NUM_DATA_BLOCKS FS_NUM_INODES\n");
        exit(1);
    }

    printf("Verifying blockify works properly...\n");
    testBlockify();

    UINT nDBlks = atoi(argv[1]);
    UINT nINodes = atoi(argv[2]);
    
    //initialize dummy file system
    FileSystem fs;
    printf("Initializing file system with initfs...\n");
    UINT succ = l2_initfs(nDBlks, nINodes, &fs);
    if(succ == 0) {
        printf("initfs succeeded with filesystem size: %d\n", fs.nBytes);
    }
    else {
        printf("Error: initfs failed with error code: %d\n", succ);
    }
    
    printf("\n==== Filesystem initial state ====\n");
    printf("\nSuperblock:\n");
    printSuperBlock(&fs.superblock);
    printf("\nINodes:\n");
    printINodes(&fs);
    printf("\nData blocks:\n");
    printDBlks(&fs);
    printf("\nFree inode cache:\n");
    printFreeINodeCache(&fs.superblock);
    printf("\nFree dblk cache:\n");
    printFreeDBlkCache(&fs.superblock);
    
    //allocate everything in filesystem
    for(int i = 0; i < nDBlks; i++) {
        allocDBlk(&fs);
    }
    
    for(int i = 0; i < nINodes - 1; i++) {
        INode testINode;
        allocINode(&fs, &testINode);
    }

    printf("\n==== Filesystem state before unmount ====\n");
    printf("\nSuperblock:\n");
    printSuperBlock(&fs.superblock);
    printf("\nINodes:\n");
    printINodes(&fs);
    printf("\nData blocks:\n");
    printDBlks(&fs);
    printf("\nFree inode cache:\n");
    printFreeINodeCache(&fs.superblock);
    printf("\nFree dblk cache:\n");
    printFreeDBlkCache(&fs.superblock);

    //unmount the current file system to disk
    l2_unmount(&fs);

    //open disk file and try mount
    printf("\nAttempting filesystem mount from disk file...\n");
    FileSystem fs_mounted;
    UINT msucc = l2_mount(&fs_mounted);
    if(msucc == 0) {
        printf("Successfully mounted filesystem!\n");
    }
    else {
        printf("Error: mount failed with error code: %d\n", msucc);
    }
    
    printf("\n==== Filesystem state after mount ====\n");
    
    printf("\nSuperblock:\n");
    printSuperBlock(&fs_mounted.superblock);
    printf("\nINodes:\n");
    printINodes(&fs_mounted);
    printf("\nData blocks:\n");
    printDBlks(&fs_mounted);
    printf("\nFree inode cache:\n");
    printFreeINodeCache(&fs_mounted.superblock);
    printf("\nFree dblk cache:\n");
    printFreeDBlkCache(&fs_mounted.superblock);
    
    assert(fs_mounted.nBytes == fs.nBytes);
    assert(fs_mounted.diskINodeBlkOffset == fs.diskINodeBlkOffset);
    assert(fs_mounted.diskDBlkOffset == fs.diskDBlkOffset);
    
    assert(memcmp(&fs_mounted.superblock, &fs.superblock, sizeof(SuperBlock)) == 0);
    
    #endif
    return 0;
}

void testBlockify() {
    #ifdef DEBUG
    SuperBlock sb;

    sb.nDBlks = 10000;
    sb.nFreeDBlks = 1000;
    sb.pFreeDBlksHead = 100;
    sb.pNextFreeDBlk = 20;
    sb.nINodes = 100;
    sb.nFreeINodes = 10;
    sb.pNextFreeINode = 2;
    sb.modified = true;

    for(UINT i = 0; i < FREE_INODE_CACHE_SIZE; i++) {
        sb.freeINodeCache[i] = i;
    }

    printf("\nSuperblock:\n");
    printSuperBlock(&sb);

    BYTE buf[BLK_SIZE];
    blockify(&sb, buf);
    SuperBlock sb_unblockified;
    unblockify(buf, &sb_unblockified);

    printf("\nSuperblock unblockified:\n");
    printSuperBlock(&sb_unblockified);

    assert(sb_unblockified.nDBlks == sb.nDBlks);
    assert(sb_unblockified.nFreeDBlks == sb.nFreeDBlks);
    assert(sb_unblockified.pFreeDBlksHead == sb.pFreeDBlksHead);
    assert(sb_unblockified.pNextFreeDBlk == sb.pNextFreeDBlk);
    assert(sb_unblockified.nINodes == sb.nINodes);
    assert(sb_unblockified.nFreeINodes == sb.nFreeINodes);
    assert(sb_unblockified.pNextFreeINode == sb.pNextFreeINode);
    assert(sb_unblockified.modified == false);

    for(UINT i = 0; i < FREE_INODE_CACHE_SIZE; i++) {
        assert(sb_unblockified.freeINodeCache[i] == sb.freeINodeCache[i]);
    }
    #endif
}
