/*
 * Set up the file system
 * By Yan
 */

#include "string.h"
#include "FileSystem.h"

/*
 * put and get need to be modified at Layer 2
 * By Yan
 */
ErrorCode put(FileSystem *fs, size_type block_no, void* buffer){
    writeBlock(&(fs->disk_emulator), block_no, buffer);

    return Success;
}

ErrorCode get(FileSystem *fs, size_type block_no, void* buffer){
    readBlock(&(fs->disk_emulator), block_no, buffer);
    
    return Success;
}

ErrorCode setDataBlockFreeList(FileSystem* fs){
    //get the block number for data block 0
    size_type first_data_block;
    getFirstDataBlockNum(&(fs->super_block), &first_data_block);

//    printf("%lu\n", fs->super_block.numOfFreeBlocks);
    //put free list onto disk
    size_type i;
    for(i=0; i<fs->super_block.numOfFreeBlocks; ){

        FreeListNode list_node;
        initFreeListNode(&list_node, i);
        put(fs, i+first_data_block, &list_node);
//        printf("%lu\n", i);
        i += FREE_BLOCK_ARRAY_SIZE+1;
    }

    //save free list head/tail block no in super block
    fs->super_block.pDataFreeListHead = first_data_block;
    fs->super_block.pDataFreeListTail = i - FREE_BLOCK_ARRAY_SIZE -1;

    return Success;
}

/*
 *size -- System size
 *percen -- percentage of inode space on disk, e.g. 10 means 10%
 *fs -- in memory buffer for files ystem 
 */
ErrorCode initFS(size_type size, size_type percen, FileSystem* fs){
    //test precondition
    
    //set up super block
    initSuperBlock(size, percen, &(fs->super_block));
  
    initDisk(&(fs->disk_emulator), fs->super_block.systemSize);
    //put everything on disk
    put(fs, SUPER_BLOCK_OFFSET, &(fs->super_block));

    //set up free list for data block
    setDataBlockFreeList(fs);
    
    return Success;
}

ErrorCode readSuperBlock(FileSystem* fs, SuperBlock* super_block){
    BYTE* buffer = malloc(BLOCK_SIZE);
    get(fs, SUPER_BLOCK_OFFSET, buffer);
    super_block = (SuperBlock*) buffer;

    return Success;
}
