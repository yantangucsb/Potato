/*
 * Freee list node interfaces
 * By Yan
 */

#include "FreeListNode.h"

ErrorCode initFreeListNode(FreeListNode* list_node, size_type block_no){
    list_node->next_node = block_no + FREE_BLOCK_ARRAY_SIZE;
    int i;
    for(i=0; i<FREE_BLOCK_ARRAY_SIZE; i++){
        list_node->free_node_arr[i] = i+block_no;
    }

    return Success;
}
