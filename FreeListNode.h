/*
 * Free list node for data block free list
 * By Yan
 */
#include "Parameters.h"

typedef struct {
    size_type next_node;
    size_type free_node_arr[FREE_BLOCK_ARRAY_SIZE];
} FreeListNode;

//only called once when setting up the system
ErrorCode initFreeListNode(FreeListNode* list_node, size_type block_no);