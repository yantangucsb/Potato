/*
 * This file implements mount, unmount, mknod, unlink, mkdir, readdir, chmod
 * By: Peng
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "SysCall_peng.h"

// mounts a filesystem from a device
ErrorCode Potato_mount(FileSystem* fs){
	BYTE buffer[BLOCK_SIZE];
	
	INT diskFile = Potato_open(DISK_PATH, O_RDWR, 0666);
	if (diskFile == -1)
		fprintf(stderr, "Error: disk open error %s\n", strerror(errno));
	lseek(diskFile, SUPERBLOCK_OFFSET, SEEK_SET);
	uint32_t bytesRead = read(diskFile, dsb, BLK_SIZE);
	if(bytesRead < BLK_SIZE) {
		fprintf(stderr, "Error: failed to read superblock from disk!\n");
		return -1;
	}
	close(diskFile);
	//INT unblockify(BYTE* buf, SuperBlock* superblock)
	//defined in superblock.c
	unblockify(dsb, &fs->superblock);
	
	//initialize filesystem parameters
	fs->nBytes = (BLK_SIZE + fs->superblock.nINodes * INODE_SIZE + fs->superblock.nDBlks * BLK_SIZE);
	fs->diskINodeBlkOffset = SUPERBLOCK_OFFSET + 1;
	fs->diskDBlkOffset = fs->diskINodeBlkOffset + fs->superblock.nINodes / INODES_PER_BLK;
	//initialize the datablk cache
	initDBlkCache(&fs->dCache);
	
	fs->disk = malloc(sizeof(DiskArray));
	openDisk(fs->disk, fs->nBytes);
	
	readDBlk(fs, fs->superblock.pFreeDBlksHead, (BYTE*) (fs->superblock.freeDBlkCache));
	//initialize inode table cache
	initOpenFileTable(&fs->openFileTable);
	initINodeTable(&fs->inodeTable);
	initINodeCache(&fs->inodeCache);
	return 0;
}

// unmounts a filesystem into a device
ErrorCode Potato_unmount(FileSystem* fs){
	 //write free block cache back to disk
	 writeDBlk(fs, fs->superblock.pFreeDBlksHead, (BYTE*) (fs->superblock.freeDBlkCache));
	 
	 //write superblock to disk
	 BYTE superblockBuf[BLK_SIZE];
	 blockify(&fs->superblock, superblockBuf);
	 writeBlk(fs->disk, SUPERBLOCK_OFFSET, superblockBuf);
	 fs->superblock.modified = false;
	 
	 //close disk to prevent future writes
	 closefs(fs);
	 
	 return 0;
}

// makes a new file
ErrorCode Potato_mknod(FileSystem* fs, char* path, uid_t uid, gid_t gid){
	INT id; // the inode id associated with the new directory
	INT par_id; // the inode id of the parent directory
	char par_path[MAX_PATH_LEN];
	
	//check if the directory already exist
	if (strcmp(path, "/") == 0) {
		fprintf(stderr, "Error: cannot create root directory outside of initfs!\n");
		return -1;
	}
	INT rRes = l2_namei(fs, path);
	if (rRes == -ENOTDIR ) {
        _err_last = _fs_NonDirInPath;
        THROW(__FILE__, __LINE__, __func__);
        return rRes;
    }
    if (rRes > 0) {
        fprintf(stderr, "Error: file or directory %s already exists!\n", path);
        return -EEXIST;
    }
    // find the last recurrence of '/'
    char *ptr;
    int ch = '/';
    ptr = strrchr(path, ch);
    
    // ptr = "/dir_name"
    char *dir_name = strtok(ptr, "/");
   
    strncpy(par_path, path, strlen(path) - strlen(ptr));
    par_path[strlen(path) - strlen(ptr)] = '\0';

    // special case for root
    if(strcmp(par_path, "") == 0) {
        //printf("its parent is root\n");
        strcpy(par_path, "/");
    }
    
    // find the inode id of the parent directory 
    par_id = l2_namei(fs, par_path);

    // check if the parent directory exists
    if(par_id < 0) {
        fprintf(stderr, "Parent directory %s is invalid or doesn't exist!\n", par_path);
        return par_id;
    }

    INode par_inode;
    INode inode;
    
    // read the parent inode
    if(readINode(fs, par_id, &par_inode) == -1) {
        fprintf(stderr, "fail to read parent directory inode %d\n", par_id);
        return -1;
    }
    
    //weilong: check for max_file_in_dir
    if (par_inode._in_filesize >= MAX_FILE_NUM_IN_DIR * sizeof(DirEntry)) {
        _err_last = _in_tooManyEntriesInDir;
        THROW(__FILE__, __LINE__, __func__);
        return -ENOSPC;
    }
    
    //weilong: check for long names 
    if (strlen(dir_name) > FILE_NAME_LENGTH) {
        _err_last = _in_fileNameTooLong;
        THROW(__FILE__, __LINE__, __func__);
        return -ENAMETOOLONG;
    }
    
    // allocate a free inode for the new file 
    id = allocINode(fs, &inode); 

    if(id == -1) {
        fprintf(stderr, "fail to allocate an inode for the new directory!\n");
        return -EDQUOT;
    }

    // insert new file entry into parent directory list
    DirEntry newEntry;
    strcpy(newEntry.key, dir_name);
    newEntry.INodeID = id;

    UINT offset;
    for(offset = 0; offset < par_inode._in_filesize; offset += sizeof(DirEntry)) {
        // search parent directory table
        DirEntry parEntry;
        readINodeData(fs, &par_inode, (BYTE*) &parEntry, offset, sizeof(DirEntry));
        
        // empty directory entry found, overwrite it
        if (parEntry.INodeID == -1){
            break;
        }
    }
    
    LONG bytesWritten = writeINodeData(fs, &par_inode, (BYTE*) &newEntry, offset, sizeof(DirEntry));
    if(bytesWritten != sizeof(DirEntry)) {
        fprintf(stderr, "Error: failed to write new entry into parent directory!\n");
        return -1;
    }

    // update parent directory file size, if it changed
    if(offset + bytesWritten > par_inode._in_filesize) {
        par_inode._in_filesize = offset + bytesWritten;
        writeINode(fs, par_id, &par_inode);
    }

    inode._in_uid = uid;
    inode._in_gid = gid;
    struct passwd *ppwd = getpwuid(uid);
    strcpy(inode._in_owner, ppwd->pw_name);

    // change the inode type to directory
    inode._in_type = REGULAR;

    // init the mode
    inode._in_permissions = S_IFREG | 0666;

    // init link count
    inode._in_linkcount = 1;

    // update the disk inode
    writeINode(fs, id, &inode);

    return id;
}

// deletes a file or directory
ErrorCode Potato_unlink(FileSystem* fs, char* path){
    // 1. get the inode of the parent directory using l2_namei
    // 2. clears the corresponding entry in the parent directory table, write
    // inode number to -1
    // 3. write the parent inode back to disk
    // 4. decrement file inode link count, write to disk
    // 5. if file link count = 0, 
    //   5.1 if file is reg file, release the inode and the data blocks
    //   5.2 if file is dir, recursively release all the concerned inodes and DBlks
    if (strcmp(path, "/") == 0) {
        fprintf(stderr, "Error: cannot unlink root directory!\n");
        return -1;
    }
    
    INT id; // the inode id of the unlinked file
    INT par_id; // the inode id of the parent directory
    char par_path[MAX_PATH_LEN];
    
    char *ptr;
    int ch = '/';

    // find the last recurrence of '/'
    ptr = strrchr(path, ch);
    strncpy(par_path, path, strlen(path) - strlen(ptr));
    par_path[strlen(path) - strlen(ptr)] = '\0';
    
    // ptr = "/node_name"
    char *node_name = strtok(ptr, "/");
    
    // special case for root
    if(strcmp(par_path, "") == 0) {
        strcpy(par_path, "/");
    }
    
    // find the inode id of the parent directory 
    par_id = (INT)l2_namei(fs, par_path);
    if(par_id < 0) { // parent directory does not exist
        fprintf(stderr, "Directory %s not found!\n", par_path);
        return par_id;
    }
     
    INode par_inode;
    INode inode;
    
    id = l2_namei(fs, path);
    if(id < 0) { // file does not exist
        fprintf(stderr, "Error: file \"%s\" not found!\n", path);
        return id;
    }
    
    // read the parent inode
    if(readINode(fs, par_id, &par_inode) == -1) {
        fprintf(stderr, "Error: fail to read parent directory inode %d\n", par_id);
        return -1;
    }

    // read the file inode
    if(readINode(fs, id, &inode) == -1) {
        fprintf(stderr, "fail to read to-be-unlinked file inode %d\n", par_id);
        return -1;
    }

    // decrement the link count of the file inode
    if(inode._in_linkcount == 0) {
        fprintf(stderr, "Error: file \"%s\" is already pending deletion (not all processes closed)!\n", path);
        return -2;
    }
    inode._in_linkcount--;
    writeINode(fs, id, &inode);
        
    UINT offset = 0;
    // free file inode when its link count is 0, which also frees the
    // associated data blocks.
    //weilong: remove dir
    if (inode._in_type == DIRECTORY) {
        // search parent directory table
        //skip . and ..
        for(offset = 2*sizeof(DirEntry); offset < inode._in_filesize; offset += sizeof(DirEntry)) {
            DirEntry entry;
            readINodeData(fs, &inode, (BYTE*) &entry, offset, sizeof(DirEntry));
            if (entry.INodeID != -1) {
                //call unlink
                char *recur_path = (char *)malloc(strlen(path) + 1 + strlen(entry.key));
                strcat(recur_path, path);
                strcat(recur_path, "/");
                strcat(recur_path, entry.key);
                #ifdef DEBUG_VERBOSE
                printf("l2_unlink continuing recursive \"rm -r %s\"\n", entry.key);
                #endif
                if (l2_unlink(fs, recur_path) != 0) {
                    _err_last = _fs_recursiveUnlinkFail;
                    THROW(__FILE__, __LINE__, __func__);
                    return -1;
                }
            }
        }
    }
    //note: the recursion occurs before the freeing step so as to not strand the children files
    //free the inode if and only if linkcount reaches 0 AND inode is not open
    if(!hasINodeEntry(&fs->inodeTable, id)) {
        freeINode(fs, id);
    }
    else {
        #ifdef DEBUG
        printf("l2_unlink found inode %d for file %s in inode table, waiting for close before freeing\n", id, path);
        #endif
    }

    //remove the inode from the parent directory
    for(offset = 0; offset < par_inode._in_filesize; offset += sizeof(DirEntry)) {
        // search parent directory table
        DirEntry entry;
        readINodeData(fs, &par_inode, (BYTE*) &entry, offset, sizeof(DirEntry));
        
        // directory entry found, mark it as removed
        if (strcmp(entry.key, node_name) == 0){
            #ifdef DEBUG_VERBOSE
            printf("l2_unlink removing file from parent directory at offset: %d\n", offset);
            #endif
            //strcpy(DEntry->key, "");
            entry.INodeID = -1;
            
            // update the parent directory table
            writeINodeData(fs, &par_inode, (BYTE*) &entry, offset, sizeof(DirEntry));
        }
    }
    
    // remove the entry from the inode cache
    INodeEntry* iEntry = removeINodeCacheEntry(&fs->inodeCache, id);
    if(iEntry != NULL) {
        #ifdef DEBUG
        printf("l2_unlink removed entry id %d from inode cache\n", id);
        #endif
        free(iEntry);
    }

    return 0;    
}

// makes a new directory
ErrorCode Potato_mkdir(FileSystem* fs, char* path, uid_t uid, gid_t gid){
	#ifdef DEBUG
    printf("l2_mkdir called for path: %s\n", path);
    #endif
    
    LONG id; // the inode id associated with the new directory
    LONG par_id; // the inode id of the parent directory
    char par_path[MAX_PATH_LEN];

    //check if the directory already exist
    if (strcmp(path, "/") == 0) {
        fprintf(stderr, "Error: cannot create root directory outside of initfs!\n");
        return -1;
    }
    LONG rRes = l2_namei(fs, path);
    if (rRes == -ENOTDIR) {
        _err_last = _fs_NonDirInPath;
        THROW(__FILE__, __LINE__, __func__);
        return -ENOTDIR;
    }
    if (rRes > 0) {
        fprintf(stderr, "Error: file or directory %s already exists!\n", path);
        return -EEXIST;
    }

    // find the last recurrence of '/'
    char *ptr;
    int ch = '/';
    ptr = strrchr(path, ch);
    
    // ptr = "/dir_name"
    char *dir_name = strtok(ptr, "/");
   
    strncpy(par_path, path, strlen(path) - strlen(ptr));
    par_path[strlen(path) - strlen(ptr)] = '\0';

    // special case for root
    if(strcmp(par_path, "") == 0) {
        //printf("its parent is root\n");
        strcpy(par_path, "/");
    }
    
    // find the inode id of the parent directory
    par_id = l2_namei(fs, par_path);
    #ifdef DEBUG_VERBOSE
    printf("l2_mkdir found parent directory inode id: %d\n", par_id);
    #endif

    // check if the parent directory exists
    if(par_id < 0) {
	_err_last = _fs_NonExistFile;
	THROW(__FILE__, __LINE__, __func__);
        return par_id;
    }

    INode par_inode;
    INode inode;

    // read the parent inode
    if(readINode(fs, par_id, &par_inode) == -1) {
        fprintf(stderr, "fail to read parent directory inode %d\n", par_id);
        return -1;
    }
  
    //weilong: check for max_file_in_dir
    if (par_inode._in_filesize >= MAX_FILE_NUM_IN_DIR * sizeof(DirEntry)) {
	_err_last = _in_tooManyEntriesInDir;
	THROW(__FILE__, __LINE__, __func__);
	return -ENOSPC;
    }


    if (strlen(dir_name) > FILE_NAME_LENGTH) {
	_err_last = _in_fileNameTooLong;
	THROW(__FILE__, __LINE__, __func__);
	return -ENAMETOOLONG;
    }
 
    // allocate a free inode for the new directory 
    id = allocINode(fs, &inode); 
    #ifdef DEBUG_VERBOSE
    printf("l2_mkdir allocated inode id %d for directory %s\n", id, dir_name);
    #endif
    if(id == -1) {
        fprintf(stderr, "Error: failed to allocate an inode for the new directory!\n");
        return -EDQUOT;
    }

    // insert new directory entry into parent directory list
    DirEntry newEntry;
    strcpy(newEntry.key, dir_name);
    newEntry.INodeID = id;

    UINT offset;
    for(offset = 0; offset < par_inode._in_filesize; offset += sizeof(DirEntry)) {
        // search parent directory table
        DirEntry parEntry;
        readINodeData(fs, &par_inode, (BYTE*) &parEntry, offset, sizeof(DirEntry));
        #ifdef DEBUG_DCACHE
        printf("the dir name of this entry is %s\n", parEntry.key);
        printf("the inode id of this entry is %d\n", parEntry.INodeID);
        #endif
        
        // empty directory entry found, overwrite it
        if (parEntry.INodeID == -1){
            break;
        }
    }
    
    #ifdef DEBUG_VERBOSE
    if(offset < par_inode._in_filesize) {
        printf("l2_mkdir inserting new entry into parent directory at index %d\n", offset / sizeof(DirEntry));
    }
    else {
        printf("l2_mkdir appending new entry to parent directory at offset %d\n", offset);
    }
    #endif
    LONG bytesWritten = writeINodeData(fs, &par_inode, (BYTE*) &newEntry, offset, sizeof(DirEntry));
    if(bytesWritten != sizeof(DirEntry)) {
        fprintf(stderr, "Error: failed to write new entry into parent directory!\n");
        return -1;
    }

    // update parent directory file size, if it changed
    if(offset + bytesWritten > par_inode._in_filesize) {
        par_inode._in_filesize = offset + bytesWritten;
        writeINode(fs, par_id, &par_inode);
    }
    
    /* allocate two entries in the new directory table (. , id) and (.., par_id) */
    // init directory table for the new directory
    DirEntry newBuf[2];
   
    // insert an entry for current directory 
    strcpy(newBuf[0].key, ".");
    newBuf[0].INodeID = id;

    // special parent directory points back to parent
    strcpy(newBuf[1].key, "..");
    newBuf[1].INodeID = par_id;
    
    bytesWritten = writeINodeData(fs, &inode, (BYTE*) newBuf, 0, 2 * sizeof(DirEntry));
    if(bytesWritten < 2 * sizeof(DirEntry)) {
        fprintf(stderr, "Error: failed to allocate data blocks for new file!\n");
        return -EDQUOT;
    }

    // change the inode type to directory
    inode._in_type = DIRECTORY;
    
    inode._in_uid = uid;
    inode._in_gid = gid;
    struct passwd *ppwd = getpwuid(uid);
    strcpy(inode._in_owner, ppwd->pw_name);

    // init the mode
    inode._in_permissions = S_IFDIR | 0755;

    // init link count
    inode._in_linkcount = 1;

    // update the inode file size
    inode._in_filesize = 2 * sizeof(DirEntry);

    // update the disk inode
    writeINode(fs, id, &inode);

    return id;
}

// reads directory contents
ErrorCode Potato_readdir(FileSystem* fs, char* path, LONG offset, DirEntry* curEntry){
	INT id; // the inode of the dir
    UINT numDirEntry = 0;

    id = (INT)l2_namei(fs, path);
    
    if(id < 0) { // directory does not exist
        fprintf(stderr, "Directory %s not found!\n", path);
        return id;
    }
    else {
        INode inode;
        
        if(readINode(fs, id, &inode) == -1) {
            fprintf(stderr, "fail to read directory inode %d\n", id);
            return -1;
        }

        if(inode._in_type != DIRECTORY) {
            fprintf(stderr, "NOT a directory\n");
            return -ENOTDIR;
        }

        numDirEntry = (inode._in_filesize)/sizeof(DirEntry);

	if (numDirEntry - 1 < offset) {
	    _err_last = _fs_EndOfDirEntry;
	    THROW(__FILE__, __LINE__, __func__);
	    return -1;		
	}

	#ifdef DEBUG_VERBOSE
	printf("%d entries in cur dir %s, reading %u\n", numDirEntry, path, offset);
	#endif
        // read the directory table
        readINodeData(fs, &inode, (BYTE *)curEntry, (LONG)(offset * sizeof(DirEntry)), (LONG)sizeof(DirEntry));
    }
    return 0;
}

//change mode
ErrorCode Potato_chmod(FileSystem* fs, char* path, mode_t mode){
	//1. resolve path
    INT INodeID = l2_namei(fs, path);
    if (INodeID < 0) {
	_err_last = _fs_NonExistFile;
	THROW(__FILE__, __LINE__, __func__);
	return INodeID;
    }
    INode curINode;
    if(readINode(fs, INodeID, &curINode) == -1) {
        fprintf(stderr, "Error: fail to read inode for file %s\n", path);
        return -1;
    }
    //printf("cur mode: %x\n", curINode._in_permissions);
    //2. check uid/gid
    //3. set mode
    curINode._in_permissions = mode;
    writeINode(fs, INodeID, &curINode);
    return 0;
}
