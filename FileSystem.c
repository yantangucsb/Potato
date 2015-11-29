/*
 * Set up the file system
 * By Yan
 * Modified by Peng
 * +Adding closefs
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "string.h"
#include "FileSystem.h"

/*
 * put and get need to be modified at Layer 2
 * can be called by others to read or write blocks
 * By Yan
 */
ErrorCode put(FileSystem *fs, size_type block_no, void* buffer){
    if(block_no >= fs->super_block.systemSize/BLOCK_SIZE || block_no < 0)
        return OutOfBound;
    return writeBlock(&(fs->disk_emulator), block_no, buffer);
}

ErrorCode get(FileSystem *fs, size_type block_no, void* buffer){
    if(block_no >= fs->super_block.systemSize/BLOCK_SIZE || block_no < 0)
        return OutOfBound;
    return readBlock(&(fs->disk_emulator), block_no, buffer);
}

ErrorCode setDataBlockFreeList(FileSystem* fs){
    //    printf("%lu\n", fs->super_block.numOfFreeBlocks);
    //put free list onto disk
    size_type i;
    for(i=0; i<fs->super_block.numOfFreeBlocks; ){

        FreeListNode list_node;
        if(i+FREE_BLOCK_ARRAY_SIZE > fs->super_block.numOfFreeBlocks-1){
            initFreeListNode(&list_node, i, fs->super_block.numOfFreeBlocks-1);
        }else{
            initFreeListNode(&list_node, i, i+FREE_BLOCK_ARRAY_SIZE);
        }
        

        //        if(i == 0)
//            printFreeListNode(&list_node);
        put(fs, i+fs->super_block.firstDataBlockId, &list_node);
//        printf("%ld\n", i+fs->super_block.firstDataBlockId);
        if(i+FREE_BLOCK_ARRAY_SIZE+1 > fs->super_block.numOfFreeBlocks){
            fs->super_block.pDataFreeListTail = i;
        }
        i += FREE_BLOCK_ARRAY_SIZE+1;
    }

    //save free list head/tail block no in super block
    fs->super_block.pDataFreeListHead = 0;
    
    //    fs->super_block.pDataFreeListTail = i - FREE_BLOCK_ARRAY_SIZE -1 + first_data_block;
    //printf("tail no:%ld\n", fs->super_block.pDataFreeListTail);
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
//    printSuperBlock(&(fs->super_block));  
    initDisk(&(fs->disk_emulator), fs->super_block.systemSize);
    //set up free list for data block
    setDataBlockFreeList(fs);
   

    //put super block on disk at the end of initialization
    //what if the super block is larger than BLOCK_SIZE? Yan
    //Map superblock to superblockonDisk
    SuperBlockonDisk super_block_on_disk;
    mapSuperBlockonDisk(&(fs->super_block), &(super_block_on_disk));
    BYTE* buf = malloc(BLOCK_SIZE);
    memcpy(buf, &super_block_on_disk, sizeof(SuperBlockonDisk));
    put(fs, SUPER_BLOCK_OFFSET, buf);
    free(buf);

    //test if load correctly for super block
/*    readSuperBlock(fs);
    printf("Test readSuperBlock()\n");
    printSuperBlock(&(fs->super_block));
*/
    get(fs, fs->super_block.pDataFreeListHead+fs->super_block.firstDataBlockId, &(fs->dataBlockFreeListHeadBuf));
    //printDisk(&(fs->disk_emulator), fs->super_block.pDataFreeListHead+fs->super_block.firstDataBlockId);
//    printf("read head buf from: %ld\n", fs->super_block.pDataFreeListHead+fs->super_block.firstDataBlockId);
    get(fs, fs->super_block.pDataFreeListTail+fs->super_block.firstDataBlockId, &(fs->dataBlockFreeListTailBuf));
    return Success;
}

ErrorCode loadFS(FileSystem* fs) {
    loadDisk(&(fs->disk_emulator));
    readSuperBlock(fs);

    //set up free list buf
    get(fs, fs->super_block.pDataFreeListHead+fs->super_block.firstDataBlockId, &(fs->dataBlockFreeListHeadBuf));
    //printDisk(&(fs->disk_emulator), fs->super_block.pDataFreeListHead+fs->super_block.firstDataBlockId);
//    printf("read head buf from: %ld\n", fs->super_block.pDataFreeListHead+fs->super_block.firstDataBlockId);
    get(fs, fs->super_block.pDataFreeListTail+fs->super_block.firstDataBlockId, &(fs->dataBlockFreeListTailBuf));

    initOpenFileTable(&(fs->open_file_table));
    initInodeTable(&(fs->inode_table));

    return Success;
}

ErrorCode readSuperBlock(FileSystem* fs){
    BYTE* buffer = malloc(BLOCK_SIZE);
    get(fs, SUPER_BLOCK_OFFSET, buffer);
//    printDisk(&(fs->disk_emulator), SUPER_BLOCK_OFFSET);
    SuperBlockonDisk sb_on_disk;
    memcpy(&sb_on_disk, buffer, sizeof(SuperBlockonDisk));
    printf("sb_on_disk entry: %ld\n", sb_on_disk.freeInodeList[33]);
    mapDisk2SuperBlockinMem(&sb_on_disk, &(fs->super_block));

    free(buffer);
    return Success;
}

ErrorCode closefs(FileSystem* fs) {
    if (destroyDisk(&(fs->disk_emulator)) != Success){
    	printf("[Close File System] Error: can not destroy disk\n");
    	return Err_destroyDisk;
    }
    return Success;
}

void printFileSystem(FileSystem* fs){
    printf("-----Potato info-----\n");
    printSuperBlock(&(fs->super_block));
    printf("-----Free list head-----\n");
    printFreeListNode(&(fs->dataBlockFreeListHeadBuf));
    printf("-----Free list tail-----\n");
    printFreeListNode(&(fs->dataBlockFreeListTailBuf));
    printf("-----Potato info finished-----\n");

}
