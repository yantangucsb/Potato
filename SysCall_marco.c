#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <DataBlockAccess.h>
#include "SysCall.h"

ErrorCode Potato_bmap(FileSystem* fs, Inode* inode, size_type* offset, size_type* block_no, size_type* block_offset) {
    printf("Get block_id for offset %ld\n", *offset);

    size_type curSize = DIRECT_BLOCK_NUM*BLOCK_SIZE;
    //if the offset is in directBlock
    if(*offset < curSize){
        size_type index = *offset/BLOCK_SIZE;
        *block_no = inode->directBlock[index];
        *block_offset = *offset % BLOCK_SIZE;
        return Success;
    }
    size_type preSize = curSize;
    curSize += BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE;

    //if the offset is in singleBlock
    if(*offset < curSize){
        size_type* block = malloc(BLOCK_SIZE);
        get(fs, inode->singleBlock+fs->super_block.firstDataBlockId, block);
        size_type index = (*offset - preSize)/BLOCK_SIZE;
        *block_no = *(block+index);
        *block_offset = (*offset - preSize) % BLOCK_SIZE;
        free(block);
        return Success;
    }

    preSize = curSize;
    curSize += BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE;
    
    //if the offset is in doubleBlock
    if(*offset < curSize){
        size_type* block = malloc(BLOCK_SIZE);
        get(fs, inode->doubleBlock+fs->super_block.firstDataBlockId, block);
        size_type index = (*offset - preSize)/(BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE);
        *block_no = *(block+index);
        get(fs, *block_no+fs->super_block.firstDataBlockId, block);
        index = (*offset - preSize - index*BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE)/BLOCK_SIZE;
        *block_no = *(block+index);
        *block_offset = (*offset - preSize) % BLOCK_SIZE;
        free(block);

        return Success;
    }

    preSize = curSize;
    curSize += BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE;

    //if the offset is in tripleBlock
    if(*offset < curSize){
        size_type* block = malloc(BLOCK_SIZE);
        get(fs, inode->tripleBlock+fs->super_block.firstDataBlockId, block);
        size_type index = (*offset - preSize)/(BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE);
        printf("tripleBlock index: %ld\n", index);
        *block_no = *(block+index);
        get(fs, *block_no+fs->super_block.firstDataBlockId, block);
        preSize += index*BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE;
        index = (*offset - preSize)/(BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE);
        printf("doubleBlock index: %ld\n", index);
        *block_no = *(block+index);
        get(fs, *block_no+fs->super_block.firstDataBlockId, block);
        preSize += index*BLOCK_SIZE/sizeof(size_type)*BLOCK_SIZE;
        index = (*offset - preSize)/BLOCK_SIZE;
        printf("singleBlock index: %ld\n", index);
        *block_no = *(block+index);
        *block_offset = (*offset - preSize)%BLOCK_SIZE;
        
        free(block);
        return Success;
    }

    return OutOfBound;
}

/*
 * Convert a path to an inode
 * path_name should be absolute path from mounting point
 * Error: NoInode
 */
ErrorCode Potato_namei(FileSystem* fs, char* path_name, size_type* inode_id){
    if(path_name == 0){
        return InodeNotExist;
    }

    //check if it is open
    OpenFileEntry* entry;
    getOpenFileEntry(&(fs->open_file_table), path_name, entry);
    if(entry != NULL){
        *inode_id = entry->inodeEntry->id;
        return Success;
    }

    //start from root directory
    *inode_id = ROOT_INODE_ID;
    
    //create local path without changing parameter path_name
    char tmp_path[FILE_PATH_LENGTH];
    strcpy(tmp_path, path_name);

    //separate each directory name by /;
    char* token = strtok(tmp_path, "/");
    while(token != NULL){
        printf("Getting inode id for %s\n", token);
        
        //get inode for current directory
        Inode inode;
        if(getInode(fs, inode_id, &inode) != Success){
            return InodeNotExist;
        }

        //check the file type
        if(inode.fileType != Directory){
            return NotDirectory;
        }

        //read in all inode data section
        size_type tmp;
        BYTE* buf = (BYTE*) malloc(inode.fileSize);
        readInodeData(fs, &inode, buf, 0, inode.fileSize, &tmp);

        //Linear scan each entry to search for token
        DirEntry* dir_entry = (DirEntry*) buf;
        bool foundEntry = false;
        size_type curSize = 0;
        while(curSize < inode.fileSize){
            printf("cur file/dir name is : %s\n", (dir_entry+curSize)->key);
            if(strcmp((dir_entry+curSize)->key, token) == 0 && (dir_entry+curSize)->inodeId != -1){
                *inode_id = dir_entry->inodeId;
                foundEntry = true;
                break;
            }
            curSize += sizeof(DirEntry);
        }
        free(buf);
        if(!foundEntry){
            return InodeNotExist;
        }
        
        token = strtok(NULL, "/");
    }
    return Success;
}

bool checkPermission(bool* permission, FileOp flag){
    if(flag == READ){
        if(*permission == false)
            return false;
    }else if(flag == WRITE || flag == TRUNCATE || flag == APPEND){
        if(*(permission+1) == false)
            return false;
    }else{
        if(*permission == false || *(permission+1) == false)
            return false;
    }
    return true;
}

ErrorCode Potato_bfree(FileSystem *fs, Inode *inode, size_type file_block_id){
    ErrorCode err = Success;
    size_type offset = file_block_id * BLOCK_SIZE;
    size_type data_block_id, data_block_offset;
    err = Potato_bmap(fs, inode, &offset, &data_block_id, &data_block_offset);
    assert(err == Success);
    
    size_type cur_internal_index = file_block_id;
    
    size_type entryNum = BLOCK_SIZE / sizeof(size_type);
    size_type entryNumS = entryNum * entryNum;
    size_type entryNumD = entryNumS * entryNum;
    size_type block_buffer_t[entryNum];
    size_type block_buffer_d[entryNum];
    size_type block_buffer_s[entryNum];
    
    if(cur_internal_index >= DIRECT_BLOCK_NUM + 1 * entryNum + 1 * entryNumS) {
        size_type T_index = (cur_internal_index - DIRECT_BLOCK_NUM - 1 * entryNum - 1 * entryNumS) / entryNumD;
        assert(T_index < 1);
        size_type D_index = (cur_internal_index - DIRECT_BLOCK_NUM - 1 * entryNum - 1 * entryNumS - T_index * entryNumD) / entryNumS;
        assert(D_index < entryNum);
        size_type S_index = (cur_internal_index - DIRECT_BLOCK_NUM - 1 * entryNum - 1 * entryNumS - T_index * entryNumD - D_index * entryNumS) / entryNum;
        assert(S_index < entryNum);
        size_type S_offset = (cur_internal_index - DIRECT_BLOCK_NUM) % entryNum;
        
        size_type T_block_id = inode->tripleBlock;
        get(fs, T_block_id, block_buffer_t);
        if(block_buffer_t[D_index] == -1) {
            printf("Unallocated triple indirect buffer\n");
            return -1;
        }
        size_type D_block_id = block_buffer_t[D_index];
        get(fs, D_block_id, block_buffer_d);
        if(block_buffer_d[S_index] == -1) {
            printf("Unallocated double indirect buffer\n");
            return -1;
        }
        size_type S_block_id = block_buffer_d[S_offset];
        get(fs, S_block_id, block_buffer_s);
        
        // free the data block
        freeBlock(fs, &block_buffer_s[S_offset]);
        block_buffer_s[S_offset] = -1;
        put(fs, S_block_id, block_buffer_s);
        
        if(S_offset == 0){
            freeBlock(fs, &S_block_id);
            block_buffer_d[S_index] = -1;
            put(fs, D_block_id, block_buffer_d);
            if(S_index == 0){
                freeBlock(fs, &D_block_id);
                block_buffer_t[D_index] = -1;
                put(fs, T_block_id, block_buffer_t);
                if(D_index == 0) {
                    
                }
            }
        }

        
        
    }
    
    return err;
}

INT Potato_open(FileSystem* fs, char* path_name, FileOp flag, mode_t modes) {
    //check if the file is already open
    OpenFileEntry* file_entry = NULL;
    getOpenFileEntry(&(fs->open_file_table), path_name, file_entry);

    //if the file is open
    if(file_entry != NULL && file_entry->fileOp == flag){
        if(checkPermission(file_entry->inodeEntry->inode.ownerPermission, flag) == false){
                printf("%s: Not enough authority to open the file", path_name);
                return -1;
        }
        file_entry->ref++;
        return 0;
    }
    
    //if the file is open but operation is different
    if(file_entry != NULL){
        if(checkPermission(file_entry->inodeEntry->inode.ownerPermission, flag) == false){
                printf("%s: Not enough authority to open the file", path_name);
                return -1;
        }

        addOpenFileEntry(&(fs->open_file_table), path_name, flag, file_entry->inodeEntry);
        //increase the inode ref
        file_entry->inodeEntry->ref++;
        return 0;
    }

    //covert the path to inode_id
    size_type inode_id;
    ErrorCode err = Potato_namei(fs, path_name, &inode_id);
    if(err == InodeNotExist){
        printf("%s: No such file or directory", path_name);
    }else if(err == NotDirectory){
        printf("%s: Not a direcotry", path_name);
    }

    if(err != Success){
        return -1;
    }

    //check if the inode is in InodeTable
    InodeEntry* inode_entry = NULL;
    getInodeEntry(&(fs->inode_table), inode_id, inode_entry);
    if(inode_entry != NULL){
        if(checkPermission(inode_entry->inode.ownerPermission, flag) == false){
                printf("%s: Not enough authority to open the file", path_name);
                return -1;
        }
        addOpenFileEntry(&(fs->open_file_table), path_name, flag, inode_entry);
        inode_entry->ref++;
        return 0;
    }

    Inode inode;
    if(getInode(fs, &inode_id, &inode) != Success){
        printf("get Inode failed.");
        return -1;
    }
    //check permission
    if(checkPermission(inode.ownerPermission, flag) == false){
        printf("%s: Not enough authority to open the file", path_name);
        return -1;
    }
    addInodeEntry(&(fs->inode_table), inode_id, &inode, inode_entry);
    addOpenFileEntry(&(fs->open_file_table), path_name, flag, inode_entry);
    return 0;
}

INT Potato_read(FileSystem* fs, char* path_name, size_type offset, BYTE* buf, size_type numBytes){

    // look up open file table for an open file entry
    ErrorCode err = Success;
    OpenFileEntry* file_entry;
    getOpenFileEntry(&(fs->open_file_table), path_name, file_entry);
    if(file_entry == NULL || (file_entry->fileOp != READ && file_entry->fileOp != READWRITE)) {
        printf("%s: file read access denied!\n", path_name);
        return -1;
    }

    // retrieve inode by the open file entry
    size_type   cur_inode_id  = file_entry->inodeEntry->id;
    Inode*      cur_inode     = &(file_entry->inodeEntry->inode);

    // record access time
    int t_access = time(NULL);
    sprintf(cur_inode->fileAccessTime, "%d", t_access);

    size_type data_size;
    err = readInodeData(fs, cur_inode, buf, offset, numBytes, &data_size);
    assert(err == Success);

    err = putInode(fs, &cur_inode_id, cur_inode);
    assert(err == Success);

    return data_size;
}

INT Potato_write(FileSystem* fs, char* path_name, size_type offset, BYTE* buf, size_type numBytes) {
    ErrorCode err = Success;
    OpenFileEntry* file_entry;
    getOpenFileEntry(&(fs->open_file_table), path_name, file_entry);
    if(file_entry == NULL || (file_entry->fileOp != WRITE && file_entry->fileOp != READWRITE)) {
        printf("%s: file write access denied!\n", path_name);
        return -1;
    }

    size_type   cur_inode_id    = file_entry->inodeEntry->id;
    Inode*      cur_inode       = &(file_entry->inodeEntry->inode);

    size_type data_size;
    err = writeInodeData(fs, cur_inode, buf, offset, numBytes, &data_size); // TODO: writeInodeData
    if(err != Success){
        printf("Write file failed!\n");
        return -1;
    }

    // modify the inode to change file size information
    if(offset + data_size > cur_inode->fileSize) {
        cur_inode->fileSize = offset + data_size;
    }

    int t_modified = time(NULL);
    sprintf(cur_inode->fileModifiedTime, "%d", t_modified);
    sprintf(cur_inode->inodeModifiedTime, "%d", t_modified);

    err = putInode(fs, &cur_inode_id, cur_inode);
    assert(err == Success);
    
    return data_size;
}

INT Potato_truncate(FileSystem* fs, char* path_name, size_type newLen) {
    
    ErrorCode err = Success;
    // use namei to find the inode by the path name
    size_type inode_id;
    Inode*    cur_inode;
    err = Potato_namei(fs, path_name, &inode_id);
    if(inode_id < 0){
        printf("No such file\n"); // CAUTION: temporarily no error handling
        return inode_id;
    }

    err = getInode(fs, &inode_id, cur_inode);
    if(err != Success){
        printf("%s: Error to read the file inode\n", path_name);
        return -1;
    }
    
    if(newLen > cur_inode->fileSize) {
        // do nothing
    }
    else if (newLen == cur_inode->fileSize) {
        // do nothing
    }
    else {
        // len - bytes to be truncated
        size_type len = cur_inode->fileSize - newLen;
        
        size_type file_block_id = (cur_inode->fileSize - 1 + BLOCK_SIZE) / BLOCK_SIZE - 1;

        if((newLen - 1 + BLOCK_SIZE) / BLOCK_SIZE == (cur_inode->fileSize - 1 + BLOCK_SIZE) / BLOCK_SIZE) {
            len = 0;
        }
        else {
            len -= cur_inode->fileSize - file_block_id * BLOCK_SIZE;
            bfree(fs, &cur_inode, file_block_id); // TODO: TO BE DONE
            file_block_id--;

        }

        while(len > 0 && file_block_id >= 0) {
        	if(len < BLOCK_SIZE) {
        		len = 0;
        	} else {
        		bfree(fs, &cur_inode, file_block_id);
        		len -= BLOCK_SIZE;
        		file_block_id--;
        	}

        }
    }
    cur_inode->fileSize = newLen;
    err = putInode(fs, &inode_id, cur_inode);
    assert(err == Success);

    return 0;
}

INT Potato_close(FileSystem* fs, char* path_name, FileOp flag) {

	//retrieve open file entry
	OpenFileEntry* file_entry;
	ErrorCode err = getOpenFileEntry(&(fs->open_file_table), path_name, file_entry);
	if(NULL == file_entry) {
		printf("Error: no such file found in the open file table.\n");
		return 1;
	}
	else if(file_entry->fileOp != flag){
		printf("Close File Error: %s authorization mismatch\n", path_name);
		return -1;
	}

	InodeEntry* inode_entry = file_entry->inodeEntry;
	assert(inode_entry->ref > 0);

	inode_entry->ref--;
	INT opcount = removeOpenFileOperation(file_entry, flag); //TODO: removeOpenFileOperation
	assert(opcount == inode_entry->ref);

	// remove the entry if opcount reaches 0
	if(opcount == 0) {
		err = removeOpenFileEntry(&(fs->open_file_table), path_name);
		assert(err == Success);

		//CAUTION: if the number of link is 0, no ops here.
	}

	return 0;
}

INT Potato_rename(FileSystem* fs, char* path_name, char* new_path_name) {

	// update the open file table
	ErrorCode err;
	OpenFileEntry* file_entry;
	err = getOpenFileEntry(&(fs->open_file_table), path_name, file_entry);

	if(file_entry != NULL){
		strcpy(file_entry->filePath, new_path_name);
	}

	size_type parent_id, new_parent_id;
	char parent_path[FILE_PATH_LENGTH];
	char new_parent_path[FILE_PATH_LENGTH];
	char *ptr;
	char *new_ptr;
	int  ch = '/';

	// search the original parent path
	ptr = strrchr(path_name, ch);
	strncpy(parent_path, path_name, strlen(path_name) - strlen(ptr));
	parent_path[strlen(path_name) - strlen(ptr)] = '\0';

	// the case for root
	if(strcmp(parent_path, "") == 0) {
		strcpy(parent_path, "/");
	}

	// ptr <- "/node_name"
	char* node_name = strtok(ptr, "/");

	Potato_namei(fs, parent_path, &parent_id);
	if(parent_id == -1){ // CAUTION: need to check the return value of namei
		printf("Error: fail to read old parent dir\n");
		return -1;
	}

	Inode* parent_node;
	err = getInode(fs, &parent_id, parent_node);

	assert(err == Success);

	size_type node_id = 0;
	for(size_type i = 0; i < parent_node->fileSize; i += sizeof(DirEntry)){
		DirEntry entry;
		size_type tmp;
		readInodeData(fs, parent_node, (BYTE*)&entry, i, sizeof(DirEntry), &tmp);

		if(strcmp(entry.key, node_name) == 0) {
			node_id = entry.inodeId;
			strcpy(entry.key, "");
			entry.inodeId = -1;

			// update the parent directory table
			writeInodeData(fs, parent_node, (size_type*)&entry, i, sizeof(DirEntry), &tmp);
		}
	}

	new_ptr = strrchr(new_path_name, ch);
	strncpy(new_parent_path, new_path_name, strlen(new_path_name) - strlen(new_ptr));
	new_parent_path[strlen(new_path_name) - strlen(new_ptr)] = '\0';

	// new_ptr = "/new_node_name"
	char * new_node_name = strtok(new_ptr, "/");

	if(strcmp(new_parent_path, "") == 0) {
		strcpy(new_parent_path, "/");
	}

	Potato_namei(fs, new_parent_path, &new_parent_id);
	Inode* new_parent_node;
	err = getInode(fs, &new_parent_id, new_parent_node);
	assert(err == Success);

	DirEntry new_entry;
	strcpy(new_entry.key, new_node_name);
	new_entry.inodeId = node_id;

	size_type entry_index;
	for(entry_index = 0; entry_index < new_parent_node->fileSize; entry_index += sizeof(DirEntry)){
		DirEntry parent_entry;
		size_type tmp;
		readInodeData(fs, new_parent_node, (BYTE*)&parent_entry, entry_index, sizeof(DirEntry), &tmp);

		if(parent_entry.inodeId == -1){ // CAUTION: -1 stands for empty node
			break;
		}
	}

	size_type bytes_written;
	writeInodeData(fs, new_parent_node, (BYTE*)&new_entry, entry_index, sizeof(DirEntry), &bytes_written);

	if(bytes_written != sizeof(DirEntry)) {
		printf("Error: parent dir entry write failed!\n");
		return -1;
	}

	if(entry_index + bytes_written > new_parent_node->fileSize) {
		new_parent_node->fileSize = entry_index + bytes_written;
		//putInode(fs, *new_parent_id, new_parent_node);
	}

	putInode(fs, &new_parent_id, new_parent_node);

	return 0;
}
