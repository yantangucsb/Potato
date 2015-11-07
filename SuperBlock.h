/*
 * This is the superblock
 * By Yan
 * Modified by Peng
 * +numOfInodes,insertInodeIndex
 */

#include <stdbool.h>
#include "Inode.h"

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

    //block no for head of the data block free list
    size_type pDataFreeListHead;

    //block no for tail of the data block free list
    size_type pDataFreeListTail;

    //the size of inode list in terms of bytes
    size_type inodeListSize;

    //the number of inodes in the file system
    size_type numOfInodes;
    
    //the number of free inodes in the file system
    size_type numOfFreeInodes;
    
    //a list of free inodes
    size_type freeInodeList[FREE_INODE_NUM];

    //the index of the next free inode in the free inode list
    size_type freeInodeIndex;
    
    //used by freeInode(), insert freed inode to free inode list using this index
    size_type insertInodeIndex;    

    //lock fields for the free block and free inode lists
    

    //a flag indicating that the super block has been modified
    bool modified;
} SuperBlock;

ErrorCode initSuperBlock(size_type size, size_type percen, SuperBlock* super_block);

ErrorCode getFirstDataBlockNum(SuperBlock* super_block, size_type* addr);

void printSuperBlock(SuperBlock* super_block);
