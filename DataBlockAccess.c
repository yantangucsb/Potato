#include "DataBlockAccess.h"

/*
 * Get a BlockId from buffer head
 * Make sure everything modified goes to disk
 */
ErrorCode allocBlock(FileSystem* fs, size_type* blockId){
    FreeListNode* headBuf = fs->dataBlockFreeListHeadBuf;
    
    //No free data block
    if(fs->super_block.numOfFreeBlocks == 0)
        return NoFreeDataBlock;
    
    //Empty Free list node should be allocated
    if(headBuf->free_block_arr[0] == -1){
        *blockId = fs->super_block.pDataFreeListHead;
        fs->super_block.pDataFreeListHead = headBuf.next_node;
        if(headBuf.next_node != -1){
            get(fs, fs->super_block.pDataFreeListHead, &(fs->dataBlockFreeListHeadBuf));
        }
        fs->numOfFreeBlocks --;
        fs->super_block.modified = true;
        return Success;
    }
    
    int i=0;
    for(i = FREE_BLOCK_ARRAY_SIZE-1; i>=0; i--){
        if(headBuf->free_block_arr[i] > -1){
            *blockId = headBuf->free_block_arr[i];
            headBuf->free_block_arr[i] = -1;
            break;
        }
    }

    fs->numOfFreeBlocks--;
    fs->super_block.modified = true;
    
    return Success;
}

ErrorCode freeBlock(FileSystem* fs, size_type* blockId){
    size_type numOfDataBlock = (fs->super_block->systemSize - BLOCK_SIZE - fs->super_block->inodeListSize)/BLOCK_SIZE;

    if(*blockId < 0 || *blockId >= numOfDataBlock)
        return OutOfBound;

    //First free block
    if(fs->super_block.pDataFreeListHead == -1){
        FreeListNode list_node;
        initEmptyNode(&list_node);
        fs->super_block.pDataFreeListHead = *blockId;
        fs->super_block.pDataFreeListTail = *blockId;
        put(fs, 
        setFreeListBuf(fs->dataBlockFreeListHeadBuf, 
    }
}

ErrorCode getBlock(FileSystem* fs, size_type* blockId){

}

ErrorCode putBlock(FileSystem* fs, size_type* blockId){

}

void PrintInfo(FileSystem* fs, size_type* blockId){

}
