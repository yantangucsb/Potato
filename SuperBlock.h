/*
 * This is the superblock
 * By Yan
 */

#include <stdbool.h>
#include "Parameters.h"

typedef struct{
    
    //the size of the file system in bytes
    size_type systemSize;

    //the number of free blocks
    size_type numOfFreeBlocks;

    /*
     * the old cache mechanism for data block
    //a list of free blocks available
    addr_type freeBlockList[BLOCK_NUM];
    
    //the index of the next free block in the free block list
    size_type freeBlockIndex;
    */

    //addr for head of the data block free list
    addr_type pdataBlockFreeListHead;

    //addr for tail of the data block free list
    addr_type pdataBlockFreeListTail;

    //the size of inode list in terms of bytes
    size_type inodeListSize;

    //the number of free inodes in the file system
    size_type numOfFreeInodes;
    
    //a list of free inodes
    addr_type freeInodeList[FREE_INODE_NUM];

    //the index of the next free inode in the free inode list
    size_type freeInodeIndex;

    //lock fields for the free block and free inode lists
    

    //a flag indicating that the super block has been modified
    bool modified;
} SuperBlock;

ErrorCode initSuperBlock(size_type size, size_type percen, SuperBlock* super_block);
