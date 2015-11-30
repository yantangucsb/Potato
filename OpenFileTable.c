#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "OpenFileTable.h"

ErrorCode initOpenFileTable(OpenFileTable* open_file_table){
    open_file_table->nOpenFiles = 0;
    open_file_table->head = NULL;

    return Success;
}

ErrorCode initInodeTable(InodeTable* inode_table){
    inode_table->nInodes = 0;
    inode_table->head = NULL;
    return Success;
}

OpenFileEntry* getOpenFileEntry(OpenFileTable* open_file_table, char* path_name){
    OpenFileEntry* open_file_entry = open_file_table->head;
    while(open_file_entry != NULL){
        if(strcmp(open_file_entry->filePath, path_name) == 0)
            return open_file_entry;
        open_file_entry = open_file_entry->next;
    }
    return NULL;

}

ErrorCode addOpenFileEntry(OpenFileTable* open_file_table, char* path_name, FileOp file_op, InodeEntry** inode_entry) {
    //create a new entry
    OpenFileEntry* new_entry = malloc(sizeof(OpenFileEntry));
    strcpy(new_entry->filePath, path_name);
    new_entry->fileOp = file_op;
    new_entry->inodeEntry = *inode_entry;
    new_entry->ref=1;

    //maintain the open file table
    new_entry->next = open_file_table->head;
    open_file_table->head = new_entry;
    open_file_table->nOpenFiles++;

    return Success;
}

InodeEntry* getInodeEntry(InodeTable* inode_table, size_type inode_id){
    InodeEntry* cur_entry = inode_table->head;
    while(cur_entry != NULL){
        if(cur_entry->id == inode_id)
            return cur_entry;
        cur_entry = cur_entry->next;
    }
    return NULL;
}

ErrorCode addInodeEntry(InodeTable* inode_table, size_type inode_id, Inode* inode, InodeEntry** inode_entry){
    //create an entry
    *inode_entry = malloc(sizeof(InodeEntry));
    (*inode_entry)->id = inode_id;
    (*inode_entry)->ref = 1;
    memcpy(&((*inode_entry)->inode), inode, sizeof(Inode));
    
    //maintain the inode table 
    inode_table->nInodes ++;
    (*inode_entry)->next = inode_table->head;
    inode_table->head = (*inode_entry);
    
    if((*inode_entry) != NULL)
        printf("[Potato_open] inode entry id: %ld\n", (*inode_entry)->id);
    else
        printf("[Potato_open] NULL ponter!\n");
    return Success;
}


ErrorCode hasINodeEntry(InodeTable* inode_table, size_type inode_id)
{
	InodeEntry* cur_entry = inode_table->head;
	while(cur_entry != NULL){
		if(cur_entry->id == inode_id)
			return Success;
		cur_entry = cur_entry->next;
	}
	return Err_hasINodeEntry;
}

ErrorCode removeOpenFileEntry(OpenFileTable* open_file_table, char* path_name) {
    OpenFileEntry* prev_entry = NULL;
    OpenFileEntry* cur_entry = open_file_table->head;
    while(cur_entry != NULL){
        if(strcmp(cur_entry->filePath, path_name) == 0) {
            if(prev_entry == NULL) {
                open_file_table->head = cur_entry->next;
            }
            else {
                prev_entry->next = cur_entry->next;
            }
            free(cur_entry);
            open_file_table->nOpenFiles--;
            return Success;
        }
        prev_entry = cur_entry;
        cur_entry = cur_entry->next;
    }
    printf("Error: open file entry not found!\n");
    return NoFreeDataBlock; // need to assign a new enum entry, however, it works anyway.
}

ErrorCode removeInodeEntry(InodeTable* inode_table, size_type id){
    if(inode_table->head != NULL && inode_table->head->id == id){
        InodeEntry* tmp = inode_table->head;
        inode_table->head = inode_table->head->next;
        free(tmp);
        return Success;
    }
    if(inode_table->head == NULL)
        return InodeNotExist;
    InodeEntry* pre = inode_table->head;
    InodeEntry* entry = inode_table->head->next;
    while(entry != NULL){
        if(entry->id == id){
            pre->next = entry->next;
            free(entry);
            return Success;
        }
        pre =entry;
        entry = entry->next;
    }
    return InodeNotExist;
}

void freeOpenFileTable(OpenFileTable* open_file_table){
    OpenFileEntry* cur_entry = open_file_table->head;
    while(cur_entry != NULL){
        OpenFileEntry* tmp = cur_entry;
        cur_entry = cur_entry->next;
        free(tmp);
    }
    open_file_table -> head = NULL;
    open_file_table->nOpenFiles = 0;
}

void freeInodeTable(InodeTable* inode_table){
    InodeEntry* cur_entry = inode_table->head;
    while(cur_entry != NULL){
        InodeEntry* tmp = cur_entry;
        cur_entry = cur_entry->next;
        free(tmp);
    }
    inode_table->head = NULL;
    inode_table->nInodes = 0;
}

void printOpenFileTable(OpenFileTable* open_file_table){
    OpenFileEntry* cur_entry = open_file_table->head;
    printf("-----Open File Table, size: %ld -----\n", open_file_table->nOpenFiles);
    while(cur_entry != NULL){
        printf("Path: %s, ",  cur_entry->filePath);
        printf("fileop: %d, ", cur_entry->fileOp);
        printf("ref: %ld, ", cur_entry->ref);
        printf("offset: %ld, ", cur_entry->offset);
        printf("inode id: %ld, ", cur_entry->inodeEntry->id);
        printf("inode ref: %ld\n", cur_entry->inodeEntry->ref);
        cur_entry = cur_entry->next;
    }
    printf("-----End Open File Table -----\n");
}

void printInodeTable(InodeTable* inode_table){
    InodeEntry* cur_entry = inode_table->head;
    printf("-----Inode Table, size: %ld -----\n", inode_table->nInodes);
    while(cur_entry != NULL){
        printf("id: %ld, ref: %ld\n", cur_entry->id, cur_entry->ref);
        cur_entry = cur_entry->next;
    }
    printf("-----End Inode Table -----\n");
}
