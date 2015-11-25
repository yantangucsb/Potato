#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
        BYTE* buf = (BYTE*) malloc(inode.fileSize);
        readInodeData(fs, &inode, buf, 0, inode.fileSize);

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
