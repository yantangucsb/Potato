/*
 * Free list node for data block free list
 * By Yan
 * Modified by Maohua Zhu
 */
#include "Parameters.h"

typedef struct {
    size_type next_node;
    size_type free_block_arr[FREE_BLOCK_ARRAY_SIZE];
} FreeListNode;

//only called once when setting up the system
ErrorCode initFreeListNode(FreeListNode* list_node, size_type block_no);

ErrorCode initEmptyNode(FreeListNode* list_node);

void printFreeListNode(FreeListNode* list_node);
