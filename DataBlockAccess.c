#include <string.h>
#include "DataBlockAccess.h"

/*
 * Get a BlockId from buffer head
 * Make sure everything modified goes to disk
 */
ErrorCode allocBlock(FileSystem* fs, size_type* blockId){
    FreeListNode* headBuf = &(fs->dataBlockFreeListHeadBuf);
    
    //No free data block
    if(fs->super_block.numOfFreeBlocks == 0)
        return NoFreeDataBlock;
    
    //Empty Free list node should be allocated
    if(headBuf->free_block_arr[0] == -1){
        *blockId = fs->super_block.pDataFreeListHead;
        fs->super_block.pDataFreeListHead = headBuf->next_node;
        if(headBuf->next_node == -1){
            fs->super_block.pDataFreeListTail = -1;
        }else{
            get(fs, fs->super_block.pDataFreeListHead+fs->super_block.firstDataBlockId, &(fs->dataBlockFreeListHeadBuf));
        }
        fs->super_block.numOfFreeBlocks --;
        fs->super_block.modified = true;
        return Success;
    }
    
    int i=0;
    for(i = FREE_BLOCK_ARRAY_SIZE-1; i>=0; i--){
        if(headBuf->free_block_arr[i] > -1){
            *blockId = headBuf->free_block_arr[i];
            headBuf->free_block_arr[i] = -1;
            if(fs->super_block.pDataFreeListHead == fs->super_block.pDataFreeListTail){
                fs->dataBlockFreeListTailBuf.free_block_arr[i] = -1;
            }
            break;
        }
    }

    fs->super_block.numOfFreeBlocks--;
    
    return Success;
}

ErrorCode freeBlock(FileSystem* fs, size_type* blockId){
    if(*blockId < 0 || *blockId >= fs->super_block.numOfDataBlocks)
        return OutOfBound;

    //First free block
    if(fs->super_block.pDataFreeListHead == -1){
        fs->super_block.pDataFreeListHead = *blockId;
        fs->super_block.pDataFreeListTail = *blockId;
        fs->super_block.modified = true;
        initEmptyNode(&(fs->dataBlockFreeListHeadBuf));
        memcpy(&(fs->dataBlockFreeListTailBuf), &(fs->dataBlockFreeListHeadBuf), sizeof(FreeListNode));
        return Success;
    }

    FreeListNode* tailBuf = &(fs->dataBlockFreeListTailBuf);
    if(tailBuf->free_block_arr[FREE_BLOCK_ARRAY_SIZE-1] > -1){
        if(fs->super_block.pDataFreeListHead != fs->super_block.pDataFreeListTail){
            put(fs, fs->super_block.pDataFreeListTail+fs->super_block.firstDataBlockId, &(fs->dataBlockFreeListTailBuf));
        }
        initEmptyNode(&(fs->dataBlockFreeListTailBuf));
        fs->super_block.pDataFreeListTail = *blockId;
        fs->super_block.numOfFreeBlocks++;
        return Success;
    }

    int i;
    for(i = 0; i <= FREE_BLOCK_ARRAY_SIZE-1; i++){
        if(tailBuf->free_block_arr[i] > -1)
            continue;
        tailBuf->free_block_arr[i] = *blockId;
        if(fs->super_block.pDataFreeListHead == fs->super_block.pDataFreeListTail){
                fs->dataBlockFreeListHeadBuf.free_block_arr[i] = *blockId;
        }
        fs->super_block.numOfFreeBlocks++;
        break;
    }

    return Success;
}
