/*
 * Test initialization of potato
 * By Yan
 */

#include <stdio.h>
#include "InodeAccess.h"
#include "DataBlockAccess.h"

int main(int argc, char *argv[]){
    FileSystem fs;
    size_type size = 0x7FFFFFF;
    printf("system size set to: %ld\n", size);
    initFS(size, 20, &fs);
    loadFS(&fs);

//    printDisk(&(fs.disk_emulator), 0);
    printf("size of Inode: %lu\n", sizeof(Inode));
    printf("size of SuperBlock: %lu\n", sizeof(SuperBlock));
//    readSuperBlock(&fs);
    printFileSystem(&fs);
    
    
    //test inode access    
/*    
    size_type inodeId;
    Inode inode;
    
    int i;
    for (i=0;i<103;i++)
    	allocInode(&fs, &inodeId, &inode);
    printf("[Get Inode] Test is Successful ~(￣▽￣)~\n");
    printf("[Initialize Inode] Test is Successful ~(￣▽￣)~\n");
    printf("[Put Inode] Test is Successful ~(￣▽￣)~\n");
    printf("[Allocate Inode] Test is Successful ~(￣▽￣)~\n");
    
    for (inodeId=0;inodeId<103;inodeId++)
    	freeInode(&fs, &inodeId);
    printf("[Free Inode] Test is Successful ~(￣▽￣)~\n");
    
    for (i=0;i<103;i++)
    	allocInode(&fs, &inodeId, &inode);
    printf("[Garbage Collection] Test is Successful ~(￣▽￣)~\n");
    
  */

    //test data block access
    size_type j;
    for (j=0; j<129; j++){
        allocBlock(&fs, &j);
        printf("Alloc data block %ld successfully.\n", j);
        printf("Current free block num: %ld\n", fs.super_block.numOfFreeBlocks);
    }

    printFreeListNode(&(fs.dataBlockFreeListHeadBuf));
    printFreeListNode(&(fs.dataBlockFreeListTailBuf));
    
    for (j=128; j>=0; j--)
        freeBlock(&fs, &j);

    printFreeListNode(&(fs.dataBlockFreeListHeadBuf));
    printFreeListNode(&(fs.dataBlockFreeListTailBuf));
    
    for (j=0; j<10; j++){
        size_type k;
        allocBlock(&fs, &k);
        printf("Alloc data block %ld successfully.\n", k);
        printf("Current free block num: %ld\n", fs.super_block.numOfFreeBlocks);
    }
    printFreeListNode(&(fs.dataBlockFreeListHeadBuf));
    printFreeListNode(&(fs.dataBlockFreeListTailBuf));
    
    for(j = 104848; j>=0; j--){
        size_type k;
        if(allocBlock(&fs, &k) == NoFreeDataBlock)
            printf("No free Blocks available.\n");
    }

    printf("Current free block num: %ld\n", fs.super_block.numOfFreeBlocks);
    printFreeListNode(&(fs.dataBlockFreeListHeadBuf));
    printFreeListNode(&(fs.dataBlockFreeListTailBuf));

    for(j = 9; j>=0; j--){
        freeBlock(&fs, &j);
        printf("Current free block num: %ld\n", fs.super_block.numOfFreeBlocks);
    }
        
    printFreeListNode(&(fs.dataBlockFreeListHeadBuf));
    printFreeListNode(&(fs.dataBlockFreeListTailBuf));
    return 0;
}
