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
#include "errno.h"
#include "pwd.h"

// mounts a filesystem from a device
ErrorCode Potato_mount(FileSystem* fs){
	//buffer for the superblock of the mounted FS
	BYTE superblk_buf[BLOCK_SIZE];
	SuperBlockonDisk* superblk_buf_pt = (SuperBlockonDisk*) superblk_buf;
	
	//int open(const char *pathname, int flags, mode_t mode);
	//A call to open() creates a new open file description, an entry in the system-wide table of open files.  The open file description records
    //the file offset and the file status flags (see below).  A file descriptor is a reference to an open file description; this reference
    //is unaffected if pathname is subsequently removed or modified to refer to a different file.
        
    //*DISK_PATH is disk partition path
    //*The argument flags must include one of the following access modes: O_RDONLY, O_WRONLY, or O_RDWR.  These request opening the file read-
    //only, write-only, or read/write, respectively.
    //*Permission 666(grant read, write access to everyone)
	INT diskFile = open(DISK_PATH, O_RDWR, 0666);
	if (diskFile == -1){
		fprintf(stderr, "[Potato mount] Error: disk open error %s\n", strerror(errno));
		return Err_OpenDisk;
	}
	
	//off_t lseek(int fd, off_t offset, int whence);
	//SEEK_SET : The offset is set to offset bytes.
	lseek(diskFile, SUPER_BLOCK_OFFSET, SEEK_SET);
	
	
	uint32_t bytesRead = read(diskFile, superblk_buf, BLOCK_SIZE);
	if(bytesRead < BLOCK_SIZE) {
		printf("[Potato mount] Error: failed to read superblock from disk!\n");
		return Err_FailReadSuperblk;
	}
	close(diskFile);
		
	//ErrorCode mapDisk2SuperBlockinMem(SuperBlockonDisk* sb_on_disk, SuperBlock* super_block)
	//defined in superblock.c
	if (mapDisk2SuperBlockinMem(superblk_buf_pt, &(fs->super_block)) != Success){
		printf("[Potato mount] Error: failed to map Disk to SuperBlock in Mem!\n");
		return Err_mapDisk2SuperBlockinMem;
	}
	
	//TODO
	//Is this correct?? to initialize disk_emulator
	//defined in FileSystem.c
	initDisk(&(fs->disk_emulator), fs->super_block.systemSize);
	//set up free list for data block
	//defined in FileSystem.c
	setDataBlockFreeList(fs);
	//set up free list buf
	//defined in FileSystem.c
	get(fs, fs->super_block.pDataFreeListHead+fs->super_block.firstDataBlockId, &(fs->dataBlockFreeListHeadBuf));
	get(fs, fs->super_block.pDataFreeListTail+fs->super_block.firstDataBlockId, &(fs->dataBlockFreeListTailBuf));
	
	//ErrorCode initOpenFileTable(OpenFileTable* open_file_table)
	//defined in FileSystem.c
	if (initOpenFileTable(&(fs->open_file_table)) != Success){
		printf("[Potato mount] Error: failed to initialize OpenFileTable!\n");
		return Err_initOpenFileTable;
	}
	//ErrorCode initInodeTable(InodeTable* inode_table)
	//defined in FileSystem.c
	if (initInodeTable(&(fs->inode_table)) != Success){
		printf("[Potato mount] Error: failed to initialize InodeTable!\n");
		return Err_initInodeTable;
	}
	
    //create root directory for mounted FileSystem
    Inode inode;
    size_type inode_id = ROOT_INODE_ID;
    if(getInode(fs, &inode_id, &inode)!=Success){
        printf("[Potato mount] Error: root directory not exist.\n");
        return Err_GetInode;
    }
	
	//TODO is this right??
    //allocate a block to root directory
    size_type block_id;
    allocBlock(fs, &block_id);
    inode.directBlock[0] = block_id;
    inode.fileType = Directory;
	
	return Success;
}

// unmounts a filesystem into a device
ErrorCode Potato_unmount(FileSystem* fs){
	//TODO
	//do we need to write free block cache back to disk
	
	//ErrorCode mapSuperBlockonDisk(SuperBlock* super_block, SuperBlockonDisk* sb_on_disk)
	//defined in superblock.c
	SuperBlockonDisk super_block_on_disk;
	if (mapSuperBlockonDisk(&(fs->super_block), &(super_block_on_disk)) != Success){
		printf("[Potato mount] Error: map SuperBlock on Disk.\n");
		return Err_mapSuperBlockonDisk;
	}
	put(fs, SUPER_BLOCK_OFFSET, &(super_block_on_disk));
		 
	fs->super_block.modified = false;
	
	//close disk to prevent future writes
	//defined in FileSystem.c
	closefs(fs);
		 
	return Success;
}

// makes a new file
ErrorCode Potato_mknod(FileSystem* fs, char* path, uid_t uid, gid_t gid, size_type* inodeId){
	size_type id; // the inode id of the mounted file system (child directory)
	size_type par_id; // the inode id of the mount point (parent directory)
	char par_path[FILE_PATH_LENGTH];
	
	//check if the directory already exist
	if (strcmp(path, "/") == 0) {
		printf("[Make Node] Error: cannot create root directory outside of initfs!\n");
		return Err_mknod;
	}
	size_type rRes;
	
	//ErrorCode Potato_namei(FileSystem* fs, char* path_name, size_type* inode_id)
	Potato_namei(fs, path, &rRes);
	
	if (rRes == -ENOTDIR ) {
//        _err_last = _fs_NonDirInPath;
//        THROW(__FILE__, __LINE__, __func__);
        *inodeId = rRes;
        return Err_mknod;
    }
    if (rRes > 0) {
        printf("[Make Node] Error: file or directory %s already exists!\n", path);
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
    
    //find the inode id of the parent directory
	//ErrorCode Potato_namei(FileSystem* fs, char* path_name, size_type* inode_id)
	Potato_namei(fs, par_path, &par_id);
	
    // check if the parent directory exists
    if(par_id < 0) {
        printf("[Make Node] Error: Parent directory %s is invalid or doesn't exist!\n", par_path);
        *inodeId = par_id;
        return Err_mknod;
    }

    Inode par_inode;
    Inode inode;
        
    // read the parent inode
    //ErrorCode getInode(FileSystem* fs, size_type* inodeId, Inode* inode)    
    if(getInode(fs, &par_id, &par_inode) == Success) {
        printf("[Make Node] Error: fail to read parent directory inode %d\n", par_id);
        return Err_mknod;
    }
    
    //check for max_file_in_dir
    if (par_inode.fileSize >= MAX_FILE_NUM_IN_DIR * sizeof(DirEntry)) {
//        _err_last = _in_tooManyEntriesInDir;
//        THROW(__FILE__, __LINE__, __func__);
        return -ENOSPC;
    }
    
    //check for long names
    if (strlen(dir_name) > FILE_NAME_LENGTH) {
//        _err_last = _in_fileNameTooLong;
//        THROW(__FILE__, __LINE__, __func__);
        return -ENAMETOOLONG;
    }
    
    //allocate a free inode for the new file
    //ErrorCode allocInode(FileSystem* fs, size_type* inodeId, Inode* inode)
    err_allocInode = allocInode(fs, &id , &inode)
    if (err_allocInode != Success){
    	printf("[Make Node] Error: fail to allocate an inode for the new directory!\n");
    	return -EDQUOT;
    }	
	
    // insert new file entry into parent directory list
    DirEntry newEntry;
    strcpy(newEntry.key, dir_name);
    newEntry.inodeId = id;

    addr_type offset;
    for(offset = 0; offset < par_inode.fileSize; offset += sizeof(DirEntry)) {
        // search parent directory table
        DirEntry parEntry;
        
        size_type readbyte;
        //ErrorCode readInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* readbyte)
        err_readInodeData = readInodeData(fs, &par_inode, (BYTE*) &parEntry, offset, sizeof(DirEntry), &readbyte);
        if (err_readInodeData != Success){
        	printf("[Make Node] Error: fail to read Inode Data!\n");
        	return err_readInodeData;
        }
        
        // empty directory entry found, overwrite it
        if (parEntry.inodeId == -1){
            break;
        }
    }
    
    size_type bytesWritten;
    //ErrorCode writeInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* writebyte)
    err_writeInodeData = writeInodeData(fs, &par_inode, (BYTE*) &newEntry, offset, sizeof(DirEntry), &bytesWritten);
    if (err_writeInodeData != Success){
    	printf("[Make Node] Error: fail to write Inode Data!\n");
    	return err_writeInodeData;
    }
    
    if(bytesWritten != sizeof(DirEntry)) {
        printf("[Make Node] Error: failed to write new entry into parent directory!\n");
        return Err_mknod;
    }

    // update parent directory file size, if it changed
    if(offset + bytesWritten > par_inode.fileSize) {
        par_inode.fileSize = offset + bytesWritten;
        
        //ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode)
        ErrorCode err_putInode = putInode(fs, &par_id, &par_inode);
        if (err_putInode != Success){
        	fprintf("[Make Node] Error: failed to put Inode!\n");
        }        
    }
	//TODO
	//DO we need to include these in the inode?
    //inode._in_uid = uid;
    //inode._in_gid = gid;
    
    struct passwd *ppwd = getpwuid(uid);
    strcpy(inode.fileOwner, ppwd->pw_name);
	
    // change the inode type to directory
    inode.fileType = Regular;
	
	//TODO
	//how t oset permission
    // init the mode
    //inode._in_permissions = S_IFREG | 0666;

    // init link count
    inode.numOfLinks = 1;

    //ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode)
    ErrorCode err_putInode = putInode(fs, &id, &inode);
    if (err_putInode != Success){
    	fprintf("[Make Node] Error: failed to put Inode!\n");
    }
    
	*inodeId = id;
    return Success;
}



// deletes a file or directory
ErrorCode Potato_unlink(FileSystem* fs, char* path, size_type* inodeId){
    // 1. get the inode of the parent directory using l2_namei
    // 2. clears the corresponding entry in the parent directory table, write
    // inode number to -1
    // 3. write the parent inode back to disk
    // 4. decrement file inode link count, write to disk
    // 5. if file link count = 0, 
    //   5.1 if file is reg file, release the inode and the data blocks
    //   5.2 if file is dir, recursively release all the concerned inodes and DBlks
    if (strcmp(path, "/") == 0) {
        printf("[Unlink] Error: cannot unlink root directory!\n");
        return Err_unlink;
    }
    
    size_type id; // the inode id of the unlinked file
    size_type par_id; // the inode id of the parent directory
    char par_path[FILE_PATH_LENGTH];
    
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
    
    Potato_namei(fs, par_path, &par_id);
    
    if(par_id < 0) { // parent directory does not exist
        printf("[Unlink] Directory %s not found!\n", par_path);
        *inodeId = par_id;
        return Err_unlink;
    }
     
    Inode par_inode;
    Inode inode;
    
    Potato_namei(fs, path, &id);
    
    if(id < 0) { // file does not exist
        printf("[Unlink] Error: file \"%s\" not found!\n", path);
        *inodeId = id;
        return Err_unlink;
    }
    
    
    // read the parent inode
    //ErrorCode getInode(FileSystem* fs, size_type* inodeId, Inode* inode)    
    if(getInode(fs, &par_id, &par_inode) == Success) {
        printf("[Unlink] Error: fail to read parent directory inode %d\n", par_id);
        return Err_unlink;
    }
    
        
    // read the file inode
    //ErrorCode getInode(FileSystem* fs, size_type* inodeId, Inode* inode)    
    if(getInode(fs, &id, &inode) == Success) {
        printf("[Unlink] Error: fail to read to-be-unlinked file inode %d\n", par_id);
        return Err_unlink;
    }    
    
	
    // decrement the link count of the file inode
    if(inode.numOfLinks == 0) {
        printf("[Unlink] Error: file \"%s\" is already pending deletion (not all processes closed)!\n", path);
        return Err_unlink;
    }
    inode.numOfLinks--;
    

	//ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode)
	ErrorCode err_putInode = putInode(fs, &id, &inode);
	if (err_putInode != Success){
		printf("[Unlink] Error: failed to put Inode!\n");
	}
    
    uint32_t offset = 0;
    // free file inode when its link count is 0, which also frees the
    // associated data blocks.
    
    //remove dir
    if (inode.fileType == Directory) {
        // search parent directory table
        //skip . and ..
        for(offset = 2*sizeof(DirEntry); offset < inode.fileSize; offset += sizeof(DirEntry)) {
            DirEntry entry;
            
		    size_type readbyte;
		    //ErrorCode readInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* readbyte)
		    ErrorCode err_readInodeData = readInodeData(fs, &inode, (BYTE*) &entry, offset, sizeof(DirEntry), &readbyte);
		    if (err_readInodeData != Success){
		    	printf("[Unlink] Error: fail to read Inode Data!\n");
		    	return err_readInodeData;
		    }
		    
            if (entry.inodeId != -1) {
                //call unlink
                char *recur_path = (char *)malloc(strlen(path) + 1 + strlen(entry.key));
                strcat(recur_path, path);
                strcat(recur_path, "/");
                strcat(recur_path, entry.key);
                printf("[Unlink] continuing recursive \"rm -r %s\"\n", entry.key);
                
                size_type return_inodeId;
                //ErrorCode Potato_unlink(FileSystem* fs, char* path, size_type* inodeId)
                ErrorCode err_unlink = Potato_unlink(fs, recur_path, &return_inodeId);
                if (err_unlink != Success) {
                	printf("[Unlink] Recursive unlink failed\n");
                    return err_unlink;
                }
            }
        }
    }
    
    
    //note: the recursion occurs before the freeing step so as to not strand the children files
    //free the inode if and only if linkcount reaches 0 AND inode is not open
    //BOOL hasINodeEntry(InodeTable* inode_table, size_type inode_id)    
    if(!hasINodeEntry(&(fs->inodeTable), id)) {
    	//ErrorCode freeInode(FileSystem* fs, size_type* inodeId)
    	ErrorCode err_freeInode = freeInode(fs, &id);
    	if (err_freeInode != Success){
    		printf("[Unlink] Free Inode failed\n");
    		return err_freeInode;
    	}
    }
    else {
        printf("Potato_unlink found inode %d for file %s in inode table, waiting for close before freeing\n", id, path);
    }
	
	
    //remove the inode from the parent directory
    for(offset = 0; offset < par_inode.fileSize; offset += sizeof(DirEntry)) {
        // search parent directory table
        DirEntry entry;
        
		size_type readbyte;
		//ErrorCode readInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* readbyte)
		//readINodeData(fs, &par_inode, (BYTE*) &entry, offset, sizeof(DirEntry));
		ErrorCode err_readInodeData = readInodeData(fs, &par_inode, (BYTE*) &entry, offset, sizeof(DirEntry), &readbyte);
		if (err_readInodeData != Success){
			printf("[Unlink] Error: fail to read Inode Data!\n");
		  	return err_readInodeData;
		}
        
        // directory entry found, mark it as removed
        if (strcmp(entry.key, node_name) == 0){
            printf("[Unlink] Error: removing file from parent directory at offset: %d\n", offset);
            //strcpy(DEntry->key, "");
            entry.inodeId = -1;
            
            // update the parent directory table
            //writeINodeData(fs, &par_inode, (BYTE*) &entry, offset, sizeof(DirEntry));
			size_type bytesWritten;
			//ErrorCode writeInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* writebyte)
			err_writeInodeData = writeInodeData(fs, &par_inode, (BYTE*) &entry, offset, sizeof(DirEntry), &bytesWritten);
			if (err_writeInodeData != Success){
				printf("[Unlink] Error: fail to write Inode Data!\n");
				return err_writeInodeData;
			}

        }
    }
    
    //InodeFreeList
    //there is no cache anymore..
    //remove the entry from the inode cache
    //INodeEntry* iEntry = removeINodeCacheEntry(&fs->inodeCache, id);
    //if(iEntry != NULL) {
    //    #ifdef DEBUG
    //    printf("l2_unlink removed entry id %d from inode cache\n", id);
    //    #endif
    //    free(iEntry);
    //}
    return Success;
}




// makes a new directory
ErrorCode Potato_mkdir(FileSystem* fs, char* path, uid_t uid, gid_t gid){
    printf("Potato_mkdir called for path: %s\n", path);
    
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
INT Potato_readdir(FileSystem* fs, char* path, LONG offset, DirEntry* curEntry){
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
INT Potato_chmod(FileSystem* fs, char* path, mode_t mode){
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
