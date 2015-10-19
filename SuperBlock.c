/*
 * set up super block
 * By Yan
 */

#include "SuperBlock.h"

ErrorCode initSuperBlock(size_type size, size_type percen, SuperBlock* super_block){
    //align the system size with BLOCK_SIZE
    size_type num_of_blocks = size/BLOCK_SIZE;
    super_block->systemSize = num_of_blocks*BLOCK_SIZE;

    //set the inode list size
    size_type num_of_inode_blocks = (size_type)(super_block->systemSize*0.01*percen)/BLOCK_SIZE;
    super_block->inodeListSize = num_of_inode_blocks*BLOCK_SIZE;

    //set num of free blocks
    super_block->numOfFreeBlocks =  (super_block->systemSize - BLOCK_SIZE - super_block->inodeListSize)/BLOCK_SIZE;

    //set num of free inodes
    size_type num_of_inodes_per_block = BLOCK_SIZE/sizeof(Inode);
    size_type num_of_free_inodes = num_of_inodes_per_block * num_of_inode_blocks;
    super_block->numOfFreeInodes = num_of_free_inodes;
    
    //initialize the free inode list cache
    super_block->freeInodeIndex = num_of_free_inodes > FREE_INODE_NUM? FREE_INODE_NUM - 1: num_of_free_inodes;
    int i = super_block->freeInodeIndex;
    for(; i>=0; i--){
        super_block->freeInodeList[i] = super_block->freeInodeIndex-i;
    }

    super_block->modified = 0;
}
