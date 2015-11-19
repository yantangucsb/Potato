/*
 * This is the superblock
 * By Yan
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

    //the number of free inodes in the file system
    size_type numOfFreeInodes;
    
    //a list of free inodes
    size_type freeInodeList[FREE_INODE_NUM];

    //the index of the next free inode in the free inode list
    size_type freeInodeIndex;

    //lock fields for the free block and free inode lists
    

    //a flag indicating that the super block has been modified
    bool modified;
} SuperBlockonDisk;

/*
 * Add some extra fields in superblock only in memeory
 */
typedef struct{
    size_type systemSize;
    size_type numOfFreeBlocks;
    //used to put blocks to disk
    size_type firstDataBlockId;
    size_type pDataFreeListHead;
    size_type pDataFreeListTail;
    size_type numOfInodes;
    size_type inodeListSize;
    size_type numOfFreeInodes;
    size_type freeInodeList[FREE_INODE_NUM];
    size_type freeInodeIndex;
    bool modified;
} SuperBlock;

ErrorCode initSuperBlock(size_type size, size_type percen, SuperBlock* super_block);

ErrorCode mapSuperBlockonDisk(SuperBlock* super_block, SuperBlockonDisk* sb_on_disk);

ErrorCode mapDisk2SuperBlockinMem(SuperBlockonDisk* sb_on_disk, SuperBlock* super_block);

void printSuperBlock(SuperBlock* super_block);
