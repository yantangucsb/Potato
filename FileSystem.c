/*
 * Set up the file system
 * By Yan
 */

#include "FileSystem.h"

/*
 *size -- System size
 *percen -- percentage of inode space on disk, e.g. 10 means 10%
 *fs -- in memory buffer for files ystem 
 */
ErrorCode setDataBlockFreeList(FileSystem* fs){
    //get the block number for data block 0
    size_type first_data_block;
    getFirstDataBlockNum(&(fs->super_block), &first_data_block);

    //put free list onto disk
    int i;
    for(i=0; i<fs->super_block.numOfFreeBlocks; ){

        FreeListNode list_node;
        initFreeListNode(&list_node, i);
        writeBlock(&(fs->disk_emulator), i+first_data_block, &list_node);
        i += FREE_BLOCK_ARRAY_SIZE+1;
    }

    //save free list head/tail block no in super block
    fs->super_block.pDataFreeListHead = first_data_block;
    fs->super_block.pDataFreeListTail = i - FREE_BLOCK_ARRAY_SIZE -1;

    return Success;
}

ErrorCode initFS(size_type size, size_type percen, FileSystem* fs){
    //test precondition
    
    //set up super block
    initSuperBlock(size, percen, &(fs->super_block));
   
    initDisk(&(fs->disk_emulator), fs->super_block.systemSize);
    //put everything on disk
    writeBlock(&(fs->disk_emulator), SUPER_BLOCK_OFFSET, &(fs->super_block));

    //set up free list for data block
    setDataBlockFreeList(fs);
    
    return Success;
}


