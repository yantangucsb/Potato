/*
 * This file implements alloc, free, ialloc, ifree.
 * By: Maohua Zhu
 */

#include "FreeListNode.h"
#include "FileSystem.h"

ErrorCode allocBlock(FileSystem* fs, addr_type* blockId) {
    ErrorCode err = Success;
    SuperBlock* super = &(fs->super_block);

    if(super->numOfFreeBlocks > 0) {
        FreeListNode* p = &(fs->dataBlockFreeListHeadBuf);

        if(p->nBlockCount == 0) {
            // alloc the free list block header itself
            *blockId = super->pDataFreeListHead;

            super->pDataFreeListHead = p->next_node;

            get(fs, super->pDataFreeListHead, &(fs->dataBlockFreeListHeadBuf));

        }
        else {
            *blockId = p->free_block_arr[p->nBlockCount - 1];
            p->nBlockCount--;
        }

        super->numOfFreeBlocks--;
    }
    else {
        err = DiskFull; // insufficient free blocks
    }
    
    return err;
}

ErrorCode freeBlock(FileSystem* fs, addr_type* blockId) {
    ErrorCode err = Success;
    SuperBlock* super = &(fs->super_block);

    if(super->numOfFreeBlocks < BLOCK_NUM) {
        FreeListNode* p = &(fs->dataBlockFreeListTailBuf);

        if(p->nBlockCount == MAX_BLOCK_NUM_PER_LIST) {
            // create new free list block header here 
            p->next_node = *blockId;

            put(fs, super->pDataFreeListTail, &(fs->dataBlockFreeListTailBuf));

            super->pDataFreeListHead = p->next_node;
            p->nBlockCount = 0;

        }
        else {
            p->free_block_arr[p->nBlockCount] = *blockId;
            p->nBlockCount++;
        }

        super->numOfFreeBlocks++;
    }
    else {
        err = OutOfBound; // more than maximum number of blocks
    }
    
    return err;
}

