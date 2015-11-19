/*
 * Free list node interfaces
 * By Yan
 */

#include <stdio.h>
#include "FreeListNode.h"

ErrorCode initFreeListNode(FreeListNode* list_node, size_type block_no){
    list_node->next_node = block_no + FREE_BLOCK_ARRAY_SIZE + 1;
    int i;
    for(i = 0; i<FREE_BLOCK_ARRAY_SIZE; i++){
        list_node->free_block_arr[i] = i + block_no + 1;
    }

    return Success;
}

ErrorCode initEmptyNode(FreeListNode* list_node) {
    list_node->next_node = -1;
    int i;
    for(i = 0; i<FREE_BLOCK_ARRAY_SIZE; i++){
        list_node->free_block_arr[i] = -1; 
    }

    return Success;
}

void printFreeListNode(FreeListNode* list_node){
    printf("-----Free List Node-----\n");
    printf("Next node no = %ld\n", list_node->next_node);
    printf("Free block array:\n");
    int i;
    for(i = 0; i < FREE_BLOCK_ARRAY_SIZE; i++){
        if(i == -1){
            //signed!!
            break;
        }
        printf("%ld ", list_node->free_block_arr[i]);
    }
    printf("\n");
}
