#include <stdlib.h>
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

ErrorCode getOpenFileEntry(OpenFileTable* open_file_table, char* path_name, OpenFileEntry* open_file_entry){
    open_file_entry = open_file_table->head;
    while(open_file_entry != NULL){
        if(strcmp(open_file_entry->filePath, path_name) == 0)
            break;
        open_file_entry = open_file_entry->next;
    }

    return Success;
}

ErrorCode addOpenFileEntry(OpenFileTable* open_file_table, char* path_name, FileOp file_op, InodeEntry* inode_entry) {
    //create a new entry
    OpenFileEntry* new_entry = malloc(sizeof(OpenFileEntry));
    strcpy(new_entry->filePath, path_name);
    new_entry->fileOp = file_op;
    new_entry->inodeEntry = inode_entry;

    //maintain the open file table
    new_entry->next = open_file_table->head;
    open_file_table->head = new_entry;
    open_file_table->nOpenFiles++;

    return Success;
}

ErrorCode getInodeEntry(InodeTable* inode_table, size_type inode_id, InodeEntry* inode_entry) {
    InodeEntry* cur_entry = inode_table->head;
    while(cur_entry != NULL){
        if(cur_entry->id == inode_id)
            break;
        cur_entry = cur_entry->next;
    }
    return Success;
}

ErrorCode addInodeEntry(InodeTable* inode_table, size_type inode_id, Inode* inode, InodeEntry* inode_entry){
    //create an entry
    inode_entry = malloc(sizeof(InodeEntry));
    inode_entry->id = inode_id;
    inode_entry->ref = 1;
    memcpy(&inode_entry->inode, inode, sizeof(Inode));
    
    //maintain the inode table 
    inode_table->nInodes ++;
    inode_entry->next = inode_table->head;
    inode_table->head = inode_entry;
    
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
    OpenFIleEntry* cur_entry = open_file_table->head;
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

