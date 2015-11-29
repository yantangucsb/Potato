#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "SysCall.h"
#include "errno.h"
#include "pwd.h"


ErrorCode Potato_bmap(FileSystem* fs, Inode* inode, size_type* offset, size_type* block_no, size_type* block_offset) {
    printf("[Potato_bmap] enter\n");
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



//by marco
ErrorCode Potato_bfree(FileSystem *fs, Inode *inode, size_type file_block_id){
	printf("[Potato_bfree] enter\n");
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
                    freeBlock(fs, &T_block_id);
                    inode->tripleBlock = -1;
                }
            }
        }
        else if(cur_internal_index >= DIRECT_BLOCK_NUM + 1 * entryNum) {
            size_type D_index = (cur_internal_index - DIRECT_BLOCK_NUM - 1 * entryNum) / entryNumS;
            assert(D_index < 1);
            size_type S_index = (cur_internal_index - DIRECT_BLOCK_NUM - 1 * entryNum - D_index * entryNumS) / entryNum;
            size_type S_offset = (cur_internal_index - DIRECT_BLOCK_NUM) % entryNum;

            
            size_type D_block_id = inode->doubleBlock;
            get(fs, D_block_id, block_buffer_d);
            if(block_buffer_d[S_index] == -1){
                printf("Unallocated double indirect buffer\n");
                return -1;
            }

            
            size_type S_block_id = block_buffer_d[S_index];
            get(fs, S_block_id, block_buffer_s);

            
            freeBlock(fs, &block_buffer_s[S_offset]);
            block_buffer_s[S_offset] = -1;
            put(fs, S_block_id, block_buffer_s);

            
            if(S_offset == 0){
                freeBlock(fs, &S_block_id);
                block_buffer_d[S_index] = -1;
                put(fs, D_block_id, block_buffer_d);
                if(S_index == 0) {
                    freeBlock(fs, &D_block_id);
                    inode->doubleBlock = -1;
                }
            }

            
        }
        else if(cur_internal_index >= DIRECT_BLOCK_NUM) {
            size_type S_index = (cur_internal_index - DIRECT_BLOCK_NUM) / entryNum;
            size_type S_offset = (cur_internal_index - DIRECT_BLOCK_NUM) % entryNum;

            
            size_type S_block_id = inode->singleBlock;
            get(fs, S_block_id, block_buffer_s);

            
            freeBlock(fs, &block_buffer_s[S_offset]);
            block_buffer_s[S_offset] = -1;
            put(fs, S_block_id, block_buffer_s);

            
            if(S_offset == 0) {
                freeBlock(fs, &S_block_id);
                inode->singleBlock = -1;
            }

            
        }
        else {
            freeBlock(fs, &inode->directBlock[cur_internal_index]);
            inode->directBlock[cur_internal_index] = -1;
        }

        

        
    }

    
    return err;
}

/*
 * Convert a path to an inode
 * path_name should be absolute path from mounting point
 * Error: NoInode
 */
INT Potato_namei(FileSystem* fs, char* path_name, size_type* inode_id){
	printf("[Potato_namei] enter\n");
    if(path_name == 0){
        return -ENOENT;
    }

    //check if it is open
    OpenFileEntry* entry;
    getOpenFileEntry(&(fs->open_file_table), path_name, entry);
    if(entry != NULL){
        *inode_id = entry->inodeEntry->id;
        return 0;
    }

    //start from root directory
    *inode_id = ROOT_INODE_ID;
    
    //create local path without changing parameter path_name
    char tmp_path[FILE_PATH_LENGTH];
    strcpy(tmp_path, path_name);

    //separate each directory name by /;
    char* token = strtok(tmp_path, "/");
    while(token != NULL){
        printf("[namei] Getting inode id for %s\n", token);
        
        //get inode for current directory
        Inode inode;
        if(getInode(fs, inode_id, &inode) != Success){
            return -ENOENT;
        }

        //check the file type
        if(inode.fileType != Directory){
            return -ENOTDIR;
        }

        //read in all inode data section
        BYTE* buf = (BYTE*) malloc(inode.fileSize);
        //modified by peng: adding a read byte argument
        size_type readbyte;
        readInodeData(fs, &inode, buf, 0, inode.fileSize, &readbyte);

        printf("[namei] file size for inode %ld: %ld\n", *inode_id, inode.fileSize);
        printf("[namei] 1st block for cur inode: %ld\n", inode.directBlock[0]); 
        //Linear scan each entry to search for token
        DirEntry* dir_entry = (DirEntry*) buf;
        bool foundEntry = false;
        size_type curSize = 0;
        while(curSize < inode.fileSize){
            printf("[namei] cur file/dir name is : %s, inode id: %ld\n", dir_entry->key, dir_entry->inodeId);
            if(strcmp(dir_entry->key, token) == 0 && dir_entry->inodeId != -1){
                *inode_id = dir_entry->inodeId;
                foundEntry = true;
                break;
            }
            curSize += sizeof(DirEntry);
            dir_entry++;
        }
        free(buf);
        if(!foundEntry){
            return -ENOENT;
        }
        
        token = strtok(NULL, "/");
    }
    return 0;
}

bool checkPermission(uint32_t permission, FileOp flag){
	printf("[checkPermission] enter\n");
	//owner's permission check
    if(flag == READ){
        if((permission/100)/4 == 0)
            return false;
    }else if(flag == WRITE || flag == TRUNCATE || flag == APPEND){
        if(((permission/100)%4)/2 == 0)
            return false;
    }else{
        if((permission/100)/4 == 0 || ((permission/100)%4)/2 == 0)
            return false;
    }
    return true;
}

INT Potato_open(FileSystem* fs, char* path_name, FileOp flag) {
	printf("[Potato_open] enter\n");
    //check if the file is already open
    OpenFileEntry* file_entry = NULL;
    getOpenFileEntry(&(fs->open_file_table), path_name, file_entry);

    //if the file is open
    if(file_entry != NULL && file_entry->fileOp == flag){
        if(checkPermission(file_entry->inodeEntry->inode.Permission, flag) == false){
                printf("%s: Not enough authority to open the file", path_name);
                return -1;
        }
        file_entry->ref++;
        return 0;
    }
    
    //if the file is open but operation is different
    if(file_entry != NULL){
        if(checkPermission(file_entry->inodeEntry->inode.Permission, flag) == false){
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
        if(checkPermission(inode_entry->inode.Permission, flag) == false){
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
    if(checkPermission(inode.Permission, flag) == false){
        printf("%s: Not enough authority to open the file", path_name);
        return -1;
    }
    addInodeEntry(&(fs->inode_table), inode_id, &inode, inode_entry);
    addOpenFileEntry(&(fs->open_file_table), path_name, flag, inode_entry);
    return 0;
}


// mounts a filesystem from a device
INT Potato_mount(FileSystem* fs){
	printf("[Potato_mount] enter\n");
	printf("[Potato mount] mount called for fs: %p\n", fs);
	printf("Reading superblock from disk...\n");
    if(loadFS(fs) != Success){
        printf("[mount] load FS failed.");
    }
/*	//buffer for the superblock of the mounted FS
	BYTE superblk_buf[BLOCK_SIZE];
	SuperBlockonDisk* superblk_buf_pt = (SuperBlockonDisk*) superblk_buf;
	
	//int open(const char *pathname, int flags, mode_t mode);
	//A call to open() creates a new open file description, an entry in the system-wide table of open files.  The open file description records
    //the file offset and the file status flags (see below).  A file descriptor is a reference to an open file description; this reference
    //is unaffected if pathname is subsequently removed or modified to refer to a different file.
        
    // *DISK_PATH is disk partition path
    // *The argument flags must include one of the following access modes: O_RDONLY, O_WRONLY, or O_RDWR.  These request opening the file read-
    //only, write-only, or read/write, respectively.
    // *Permission 666(grant read, write access to everyone)
	INT diskFile = open(DISK_PATH, O_RDWR, 0666);
	if (diskFile == -1){
		printf("[Potato mount] Error: disk open error %s\n", strerror(errno));
		//return Err_OpenDisk;
		return -1;
	}
	
	//off_t lseek(int fd, off_t offset, int whence);
	//SEEK_SET : The offset is set to offset bytes.
	lseek(diskFile, SUPER_BLOCK_OFFSET, SEEK_SET);
	
	
	uint32_t bytesRead = read(diskFile, superblk_buf, BLOCK_SIZE);
	if(bytesRead < BLOCK_SIZE) {
		printf("[Potato mount] Error: failed to read superblock from disk!\n");
		//return Err_FailReadSuperblk;
		return -1;
	}
	close(diskFile);
		
	//ErrorCode mapDisk2SuperBlockinMem(SuperBlockonDisk* sb_on_disk, SuperBlock* super_block)
	//defined in superblock.c
	if (mapDisk2SuperBlockinMem(superblk_buf_pt, &(fs->super_block)) != Success){
		printf("[Potato mount] Error: failed to map Disk to SuperBlock in Mem!\n");
		return -1;
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
		//return Err_initOpenFileTable;
		return -1;
	}
	//ErrorCode initInodeTable(InodeTable* inode_table)
	//defined in FileSystem.c
	if (initInodeTable(&(fs->inode_table)) != Success){
		printf("[Potato mount] Error: failed to initialize InodeTable!\n");
		//return Err_initInodeTable;
		return -1;
	}
*/	
/*    //create root directory for mounted FileSystem
    Inode inode;
    size_type inode_id = ROOT_INODE_ID;
    if(getInode(fs, &inode_id, &inode)!=Success){
        printf("[Potato mount] Error: root directory not exist.\n");
        //return Err_GetInode;
        return -1;
    }

	//TODO is this right??
    //allocate a block to root directory
    size_type block_id;
    allocBlock(fs, &block_id);
    inode.directBlock[0] = block_id;
    inode.fileType = Directory;
    inode.used = true;
    inode.Permission = 0x777;

    DirEntry dir_entry[2];
    strcpy(dir_entry[0], ".");
    dir_entry[0].inodeId = ROOT_INODE_ID;
    strcpy(dir_entry[1], "..");
    dir_entry[1].inodeId = ROOT_INODEI_ID;

    BYTE buf[BLOCK_SIZE];
    memcpy(buf, dir_entry, sizeof(dir_entry));
    put(fs, block_id + fs->super_block.firstDataBlockId, buf);
*/
    return 0;
}

// unmounts a filesystem into a device
INT Potato_unmount(FileSystem* fs){
	printf("[Potato_unmount] enter\n");
	//put super block on disk
    SuperBlockonDisk super_block_on_disk;
	if (mapSuperBlockonDisk(&(fs->super_block), &(super_block_on_disk)) != Success){
		printf("[Potato mount] Error: map SuperBlock on Disk.\n");
		//return Err_mapSuperBlockonDisk;
		return -1;
	}
	put(fs, SUPER_BLOCK_OFFSET, &(super_block_on_disk));
		 
    //put free list buf into disk
    put(fs, fs->super_block.pDataFreeListHead + fs->super_block.firstDataBlockId, &(fs->dataBlockFreeListHeadBuf));
    put(fs, fs->super_block.pDataFreeListTail + fs->super_block.firstDataBlockId, &(fs->dataBlockFreeListTailBuf));

    //put inode entries to disk
    InodeEntry* cur_entry = fs->inode_table.head;
    while(cur_entry != NULL){
        putInode(fs, &(cur_entry->id), &(cur_entry->inode)); 
    }

	//close disk to prevent future writes
	//defined in FileSystem.c
	closefs(fs);
		 
	return 0;
}


// makes a new file
INT Potato_mknod(FileSystem* fs, char* i_path, uid_t uid, gid_t gid){
    char *path = i_path;
	printf("[Potato_mknod] enter\n");
	size_type id; // the inode id of the mounted file system (child directory)
	size_type par_id; // the inode id of the mount point (parent directory)
	char par_path[FILE_PATH_LENGTH];
	
	//check if the directory already exist
	if (strcmp(path, "/") == 0) {
		printf("[Make Node] Error: cannot create root directory outside of initfs!\n");
		//return Err_mknod;
		return -1;
	}
	size_type rRes;
	
	//ErrorCode Potato_namei(FileSystem* fs, char* path_name, size_type* inode_id)
	INT err = Potato_namei(fs, path, &rRes);
	
	if (err == -ENOTDIR ) {
//        _err_last = _fs_NonDirInPath;
//        THROW(__FILE__, __LINE__, __func__);
        //*inodeId = rRes;
        //return Err_mknod;
        return err;
    }
    if (rRes > 0) {
        printf("[Make Node] Error: file or directory %s already exists!\n", path);
        return -EEXIST;
    }
    // find the last recurrence of '/'
    int ch = '/';
    char* ptr = strrchr(path, ch);
    
    // ptr = "/dir_name"
    printf("[Potato_mknod] prt: %s\n", ptr);
    char* dir_name = strtok(ptr, "/");
   
    strncpy(par_path, path, strlen(path) - strlen(ptr));
    par_path[strlen(path) - strlen(ptr)] = '\0';

    // special case for root
    if(strcmp(par_path, "") == 0) {
        //printf("its parent is root\n");
        strcpy(par_path, "/");
    }
    
    //find the inode id of the parent directory
	//ErrorCode Potato_namei(FileSystem* fs, char* path_name, size_type* inode_id)
	err = Potato_namei(fs, par_path, &par_id);
	
    // check if the parent directory exists
    if(err < 0) {
        printf("[Make Node] Error: Parent directory %s is invalid or doesn't exist!\n", par_path);
        //*inodeId = par_id;
        //return Err_mknod;
        return par_id;
    }

    Inode par_inode;
    Inode inode;
        
    // read the parent inode
    //ErrorCode getInode(FileSystem* fs, size_type* inodeId, Inode* inode)    
    if(getInode(fs, &par_id, &par_inode) != Success) {
        printf("[Make Node] Error: fail to read parent directory inode %ld\n", par_id);
        //return Err_mknod;
        return -1;
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
    ErrorCode err_allocInode = allocInode(fs, &id , &inode);
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
        ErrorCode err_readInodeData = readInodeData(fs, &par_inode, (BYTE*) &parEntry, offset, sizeof(DirEntry), &readbyte);
        if (err_readInodeData != Success){
        	printf("[Make Node] Error: fail to read Inode Data!\n");
        	//return err_readInodeData;
        	return -1;
        }
        
        // empty directory entry found, overwrite it
        if (parEntry.inodeId == -1){
            break;
        }
    }
    
    size_type bytesWritten;
    //ErrorCode writeInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* writebyte)
    ErrorCode err_writeInodeData = writeInodeData(fs, &par_inode, (BYTE*) &newEntry, offset, sizeof(DirEntry), &bytesWritten);
    if (err_writeInodeData != Success){
    	printf("[Make Node] Error: fail to write Inode Data!\n");
    	//return err_writeInodeData;
    	return -1;
    }
    
    if(bytesWritten != sizeof(DirEntry)) {
        printf("[Make Node] Error: failed to write new entry into parent directory!\n");
        //return Err_mknod;
        return -1;
    }

    // update parent directory file size, if it changed
    if(offset + bytesWritten > par_inode.fileSize) {
        par_inode.fileSize = offset + bytesWritten;
        
        //ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode)
        ErrorCode err_putInode = putInode(fs, &par_id, &par_inode);
        if (err_putInode != Success){
        	printf("[Make Node] Error: failed to put Inode!\n");
        	return -1;
        }        
    }
	
    inode._in_uid = uid;
    inode._in_gid = gid;
    
    struct passwd *ppwd = getpwuid(uid);
    strcpy(inode.fileOwner, ppwd->pw_name);
	
    // change the inode type to directory
    inode.fileType = Regular;
	
    inode.Permission = S_IFREG | 0666;

    // init link count
    inode.numOfLinks = 1;

    //ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode)
    ErrorCode err_putInode = putInode(fs, &id, &inode);
    if (err_putInode != Success){
    	printf("[Make Node] Error: failed to put Inode!\n");
    	return -1;
    }
    
	//*inodeId = id;
    return id;
}


// deletes a file or directory
INT Potato_unlink(FileSystem* fs, char* path){
	printf("[Potato_unlink] enter\n");
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
        //return Err_unlink;
        return -1;
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
        //*inodeId = par_id;
        return par_id;
    }
     
    Inode par_inode;
    Inode inode;
    
    Potato_namei(fs, path, &id);
    
    if(id < 0) { // file does not exist
        printf("[Unlink] Error: file \"%s\" not found!\n", path);
        //*inodeId = id;
        //return Err_unlink;
        return id;
    }
    
    
    // read the parent inode
    //ErrorCode getInode(FileSystem* fs, size_type* inodeId, Inode* inode)    
    if(getInode(fs, &par_id, &par_inode) == Success) {
        printf("[Unlink] Error: fail to read parent directory inode %ld\n", par_id);
        //return Err_unlink;
        return -1;
    }
    
        
    // read the file inode
    //ErrorCode getInode(FileSystem* fs, size_type* inodeId, Inode* inode)    
    if(getInode(fs, &id, &inode) == Success) {
        printf("[Unlink] Error: fail to read to-be-unlinked file inode %ld\n", par_id);
        //return Err_unlink;
        return -1;
    }    
    
	
    // decrement the link count of the file inode
    if(inode.numOfLinks == 0) {
        printf("[Unlink] Error: file \"%s\" is already pending deletion (not all processes closed)!\n", path);
        //return Err_unlink;
        return -2;
    }
    inode.numOfLinks--;
    

	//ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode)
	ErrorCode err_putInode = putInode(fs, &id, &inode);
	if (err_putInode != Success){
		printf("[Unlink] Error: failed to put Inode!\n");
		return -1;
	}
    
    addr_type offset = 0;
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
		    	//return err_readInodeData;
		    	return -1;
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
                //ErrorCode err_unlink = Potato_unlink(fs, recur_path, &return_inodeId);
                //if (err_unlink != Success) {
                if (Potato_unlink(fs, recur_path) != 0) {
                	printf("[Unlink] Recursive unlink failed\n");
                    //return err_unlink;
                    return -1;
                }
            }
        }
    }
    
    
    //note: the recursion occurs before the freeing step so as to not strand the children files
    //free the inode if and only if linkcount reaches 0 AND inode is not open
    //BOOL hasINodeEntry(InodeTable* inode_table, size_type inode_id)

    if (hasINodeEntry(&(fs->inode_table), id) != Success) {
    	//ErrorCode freeInode(FileSystem* fs, size_type* inodeId)
    	ErrorCode err_freeInode = freeInode(fs, &id);
    	if (err_freeInode != Success){
    		printf("[Unlink] Free Inode failed\n");
    		//return err_freeInode;
    	}
    }
    else {
        printf("Potato_unlink found inode %ld for file %s in inode table, waiting for close before freeing\n", id, path);
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
		  	//return err_readInodeData;
		  	return -1;
		}
        
        // directory entry found, mark it as removed
        if (strcmp(entry.key, node_name) == 0){
            printf("[Unlink] removing file from parent directory at offset: %d\n", offset);
            //strcpy(DEntry->key, "");
            entry.inodeId = -1;
            
            // update the parent directory table
            //writeINodeData(fs, &par_inode, (BYTE*) &entry, offset, sizeof(DirEntry));
			size_type bytesWritten;
			//ErrorCode writeInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* writebyte)
			ErrorCode err_writeInodeData = writeInodeData(fs, &par_inode, (BYTE*) &entry, offset, sizeof(DirEntry), &bytesWritten);
			if (err_writeInodeData != Success){
				printf("[Unlink] Error: fail to write Inode Data!\n");
				//return err_writeInodeData;
				return -1;
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
    return 0;
}


// makes a new directory
INT Potato_mkdir(FileSystem* fs, char* path, uid_t uid, gid_t gid){
    printf("[Potato_mkdir] enter\n");
    printf("Potato_mkdir called for path: %s\n", path);
    
    size_type id; // the inode id associated with the new directory
    size_type par_id; // the inode id of the parent directory
    char par_path[FILE_PATH_LENGTH];

    //check if the directory already exist
    if (strcmp(path, "/") == 0) {
        printf("[mkdir] Error: cannot create root directory outside of initfs!\n");
        //return Err_mkdir;
        return -1;
    }
	
    size_type rRes;
    Potato_namei(fs, path, &rRes);
    
    if (rRes == -ENOTDIR) {
//        _err_last = _fs_NonDirInPath;
//        THROW(__FILE__, __LINE__, __func__);
        return -ENOTDIR;
    }
    if (rRes > 0) {
        printf("[mkdir] Error: file or directory %s already exists!\n", path);
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
    Potato_namei(fs, par_path, &par_id);
    printf("[mkdir] Potato_mkdir found parent directory inode id: %ld\n", par_id);

    // check if the parent directory exists
    if(par_id < 0) {
//	_err_last = _fs_NonExistFile;
//	THROW(__FILE__, __LINE__, __func__);
		printf("[mkdir] Directory %s not found!\n", par_path);
		//*inodeId = par_id;
        //return Err_mkdir;
        return par_id;
    }

    Inode par_inode;
    Inode inode;
    
    
	// read the parent inode
	//readINode(fs, par_id, &par_inode) == -1
    if(getInode(fs, &par_id, &par_inode) != Success){
        printf("[mkdir] Error: fail to read parent directory inode %ld\n", par_id);
        //return Err_GetInode;
        return -1;
    }
    
    //check for max_file_in_dir
    if (par_inode.fileSize >= MAX_FILE_NUM_IN_DIR * sizeof(DirEntry)) {
//	_err_last = _in_tooManyEntriesInDir;
//	THROW(__FILE__, __LINE__, __func__);
		return -ENOSPC;
    }


    if (strlen(dir_name) > FILE_NAME_LENGTH) {
//	_err_last = _in_fileNameTooLong;
//	THROW(__FILE__, __LINE__, __func__);
		return -ENAMETOOLONG;
    }
	
    // allocate a free inode for the new directory 
    //ErrorCode allocInode(FileSystem* fs, size_type* inodeId, Inode* inode)
    ErrorCode err_allocInode = allocInode(fs, &id , &inode);
    if (err_allocInode != Success || id == -1){
    	printf("[mkdir] Error: fail to allocate an inode for the new directory!\n");
    	return -EDQUOT;
    }
    printf("[mkdir] Potato_mkdir allocated inode id %ld for directory %s\n", id, dir_name);

    // insert new directory entry into parent directory list
    DirEntry newEntry;
    strcpy(newEntry.key, dir_name);
    newEntry.inodeId = id;
	
    addr_type offset;
    for(offset = 0; offset < par_inode.fileSize; offset += sizeof(DirEntry)) {
        // search parent directory table
        DirEntry parEntry;
        
		size_type readbyte;
		//ErrorCode readInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* readbyte)
		//readINodeData(fs, &par_inode, (BYTE*) &parEntry, offset, sizeof(DirEntry));
		ErrorCode err_readInodeData = readInodeData(fs, &par_inode, (BYTE*) &parEntry, offset, sizeof(DirEntry), &readbyte);
		if (err_readInodeData != Success){
			printf("[Unlink] Error: fail to read Inode Data!\n");
		  	//return err_readInodeData;
		  	return -1;
		}
        
        printf("[mkdir] the dir name of this entry is %s\n", parEntry.key);
        printf("[mkdir] the inode id of this entry is %ld\n", parEntry.inodeId);
        
        // empty directory entry found, overwrite it
        if (parEntry.inodeId == -1){
            break;
        }
    }

    if(offset < par_inode.fileSize){
        printf("[mkdir] mkdir inserting new entry into parent directory at index %ld\n", offset / sizeof(DirEntry));
    }
    else {
        printf("[mkdir] mkdir appending new entry to parent directory at offset %d\n", offset);
    }

	//writeINodeData(fs, &par_inode, (BYTE*) &newEntry, offset, sizeof(DirEntry));
	size_type bytesWritten;
	//ErrorCode writeInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* writebyte)
	ErrorCode err_writeInodeData = writeInodeData(fs, &par_inode, (BYTE*) &newEntry, offset, sizeof(DirEntry), &bytesWritten);
	if (err_writeInodeData != Success){
		printf("[Unlink] Error: failed to write new entry into parent directory!\n");
		//return err_writeInodeData;
		return -1;
	}
	
	// update parent directory file size, if it changed
    if(offset + bytesWritten > par_inode.fileSize) {
        par_inode.fileSize = offset + bytesWritten;
        //ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode)
        //writeINode(fs, par_id, &par_inode);
        ErrorCode err_putInode = putInode(fs, &par_id, &par_inode);
        if (err_putInode != Success){
        	printf("[mkdir] Error: failed to put Inode!\n");
        	return -1;
        }
    }
    
    /* allocate two entries in the new directory table (. , id) and (.., par_id) */
    // init directory table for the new directory
    DirEntry newBuf[2];
   
    // insert an entry for current directory 
    strcpy(newBuf[0].key, ".");
    newBuf[0].inodeId = id;

    // special parent directory points back to parent
    strcpy(newBuf[1].key, "..");
    newBuf[1].inodeId = par_id;
    
	//writeINodeData(fs, &inode, (BYTE*) newBuf, 0, 2 * sizeof(DirEntry));
	//ErrorCode writeInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* writebyte)
	err_writeInodeData = writeInodeData(fs, &inode, (BYTE*) newBuf, 0, 2 * sizeof(DirEntry), &bytesWritten);
	if (err_writeInodeData != Success){
		printf("[mkdir] Error: failed to write new entry into parent directory!\n");
		//return err_writeInodeData;
		return -1;
	}
	
    if (bytesWritten < 2 * sizeof(DirEntry)) {
        printf("[mkdir] Error: failed to allocate data blocks for new file!\n");
        return -EDQUOT;
    }

    // change the inode type to directory
    inode.fileType = Directory;
    
    inode._in_uid = uid;
    inode._in_gid = gid;
    struct passwd *ppwd = getpwuid(uid);
    strcpy(inode.fileOwner, ppwd->pw_name);
	
    inode.Permission = S_IFDIR | 0755;

    // init link count
    inode.numOfLinks = 1;

    // update the inode file size
    inode.fileSize = 2 * sizeof(DirEntry);
	
	// update the disk inode
	// writeINode(fs, id, &inode);
	// ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode)
	ErrorCode err_putInode = putInode(fs, &id, &inode);
	if (err_putInode != Success){
		printf("[mkdir] Error: failed to put Inode!\n");
		return -1;
	}
	//*inodeId = id;
    return id;
}



// reads directory contents
INT Potato_readdir(FileSystem* fs, char* path, LONG offset, DirEntry* curEntry){
	printf("[Potato_readdir] enter\n");
	size_type id; // the inode of the dir
    uint32_t numDirEntry = 0;

	//ErrorCode Potato_namei(FileSystem* fs, char* path_name, size_type* inode_id)
	//id = (INT)l2_namei(fs, path);
	Potato_namei(fs, path, &id);
    
    if(id < 0) { // directory does not exist
        printf("[Readdir] Directory %s not found!\n", path);
        //*inodeId = id;
        //return Err_readdir;
        return id;
    }
    else {
        Inode i_node;
        
        //readINode(fs, id, &i_node) == -1
		if(getInode(fs, &id, &i_node) != Success){
		    printf("[Readdir] Error: fail to read directory inode %ld\n", id);
		    //return Err_readdir;
		    return -1;
		}
        
        if(i_node.fileType != Directory) {
            printf("[Readdir] Error: NOT a directory\n");
            return -ENOTDIR;
        }

        numDirEntry = (i_node.fileSize)/sizeof(DirEntry);

		if (numDirEntry - 1 < offset) {
			return -1;
		}

		printf("[Readdir] %d entries in cur dir %s, reading %ld\n", numDirEntry, path, offset);
		
        // read the directory table
        //readINodeData(fs, &i_node, (BYTE *)curEntry, (LONG)(offset * sizeof(DirEntry)), (LONG)sizeof(DirEntry));
        size_type readbyte;
        //ErrorCode readInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* readbyte)
        ErrorCode err_readInodeData = readInodeData(fs, &i_node, (BYTE*) curEntry, (LONG)(offset * sizeof(DirEntry)), (LONG)sizeof(DirEntry), &readbyte);
        if (err_readInodeData != Success){
        	printf("[Readdir] Error: fail to read Inode Data!\n");
        	return -1;
        }
    }
    return 0;
}

//change mode
INT Potato_chmod(FileSystem* fs, char* path, uint32_t set_permission){
	printf("[Potato_chmod] enter\n");
	//1. resolve path
    size_type INode_ID;
    //l2_namei(fs, path);
    Potato_namei(fs, path, &INode_ID);
    
    if (INode_ID < 0) {
		//*inodeId = INode_ID;
		//return Err_chmod;
		return INode_ID;
    }
    Inode curINode;
    
    //readINode(fs, INodeID, &curINode) == -1
	if(getInode(fs, &INode_ID, &curINode) != Success){
		printf("[Readdir] Error: fail to read inode for file %s\n", path);
		//return Err_readdir;
		return -1;
	}    
    
    //printf("cur mode: %x\n", curINode._in_permissions);
    //2. check uid/gid
    //3. set mode
    
    curINode.Permission = set_permission;


    //ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode)
    //writeINode(fs, INode_ID, &curINode);
    ErrorCode err_putInode = putInode(fs, &INode_ID, &curINode);
    if (err_putInode != Success){
    	printf("[Readdir] Error: failed to put Inode!\n");
    	return -1;
    }
	
    return 0;
}


// getattr
INT Potato_getattr(FileSystem* fs, char *path, struct stat *stbuf) {
	printf("[Potato_getattr] enter\n");
	size_type INodeID;
	//size_type INodeID = l2_namei(fs, path);
	Potato_namei(fs, path, &INodeID);
	if(INodeID < 0){ // CAUTION: need to check the return value of namei
		printf("[getattr] Error: fail to read old parent dir\n");
		return INodeID;
	}

    Inode i_node;
    //readINode(fs, INodeID, &i_node);
	printf("[Potato_getattr] inode id for %s: %ld\n", path, INodeID);
    if(getInode(fs, &INodeID, &i_node) != Success){
		printf("[getattr] Error: fail to read inode for file %s\n", path);
		//return Err_readdir;
		return -1;
	}
	
    stbuf->st_dev = 0;
    stbuf->st_ino = INodeID;
    stbuf->st_mode = i_node.Permission;
    stbuf->st_nlink = i_node.numOfLinks;
    stbuf->st_uid = i_node._in_uid;
    stbuf->st_gid = i_node._in_gid;
    stbuf->st_size = i_node.fileSize;
    stbuf->st_blksize = BLOCK_SIZE;
    stbuf->st_blocks = i_node.fileSize / BLOCK_SIZE;
    stbuf->st_atime = i_node.fileAccessTime;
    stbuf->st_mtime = i_node.fileModifiedTime;
    stbuf->st_ctime = i_node.inodeModifiedTime;
    return 0;
}

//1. resolve path and read inode
//2. check uid/gid
//3. set uid/gid and write inode
INT Potato_chown(FileSystem *fs, char *path, uid_t uid, gid_t gid){
	printf("[Potato_chown] enter\n");
	//1. resolve path
	size_type INodeID;
	//INT INodeID = l2_namei(fs, path);
	Potato_namei(fs, path, &INodeID);
	if(INodeID < 0){ // CAUTION: need to check the return value of namei
		printf("[chown] Error: fail to read dir\n");
		return INodeID;
	}
	
	Inode curINode;
	
    //readINode(fs, INodeID, &curINode) == -1
	if(getInode(fs, &INodeID, &curINode) != Success){
		printf("[chown] Error: fail to read inode for file %s\n", path);
		//return Err_readdir;
		return -1;
	}	
    
    //2. check uid/gid
    //3. set owner
    curINode._in_uid = uid;
    curINode._in_gid = gid;
    
    //ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode)
    //writeINode(fs, INodeID, &curINode);
    ErrorCode err_putInode = putInode(fs, &INodeID, &curINode);
    if (err_putInode != Success){
    	printf("[chown] Error: failed to put Inode!\n");
    	return -1;
    }
    return 0;
}

INT Potato_read(FileSystem* fs, char* path_name, size_type offset, BYTE* buf, size_type numBytes){
	printf("[Potato_read] enter\n");
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
//    int t_access = time(NULL);
//    sprintf(cur_inode->fileAccessTime, "%d", t_access);
    cur_inode->fileAccessTime = time(NULL);
    size_type data_size;
    err = readInodeData(fs, cur_inode, buf, offset, numBytes, &data_size);
    assert(err == Success);

    err = putInode(fs, &cur_inode_id, cur_inode);
    assert(err == Success);

    return data_size;
}


INT Potato_write(FileSystem* fs, char* path_name, size_type offset, BYTE* buf, size_type numBytes) {
    printf("[Potato_write] enter\n");
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
    //writeInodeData(fs, cur_inode, buf, offset, numBytes, &data_size);
	//ErrorCode writeInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* writebyte)
	ErrorCode err_writeInodeData = writeInodeData(fs, cur_inode, buf, offset, numBytes, &data_size);
	if (err_writeInodeData != Success){
		printf("Error: failed to write new entry into parent directory!\n");
		return -1;
	}	
    

    // modify the inode to change file size information
    if(offset + data_size > cur_inode->fileSize) {
        cur_inode->fileSize = offset + data_size;
    }

//    int t_modified = time(NULL);
//    sprintf(cur_inode->fileModifiedTime, "%d", t_modified);
//    sprintf(cur_inode->inodeModifiedTime, "%d", t_modified);
    cur_inode->fileModifiedTime = time(NULL);
    cur_inode->inodeModifiedTime = time(NULL);
    err = putInode(fs, &cur_inode_id, cur_inode);
    assert(err == Success);
    
    return data_size;
}



INT Potato_rename(FileSystem* fs, char* path_name, char* new_path_name) {
	printf("[Potato_rename] enter\n");
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
			//writeInodeData(fs, parent_node, (size_type*)&entry, i, sizeof(DirEntry), &tmp);
			//ErrorCode writeInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* writebyte)
			ErrorCode err_writeInodeData = writeInodeData(fs, parent_node, (BYTE*) &entry, i, sizeof(DirEntry), &tmp);
			if (err_writeInodeData != Success){
				printf("Error: failed to write new entry into parent directory!\n");
				return -1;
			}
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
	
	//writeInodeData(fs, new_parent_node, (BYTE*)&new_entry, entry_index, sizeof(DirEntry), &bytes_written);
	//ErrorCode writeInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* writebyte)
	ErrorCode err_writeInodeData = writeInodeData(fs, new_parent_node, (BYTE*)&new_entry, entry_index, sizeof(DirEntry), &bytes_written);
	if (err_writeInodeData != Success){
		printf("Error: failed to write new entry into parent directory!\n");
		return -1;
	}		
				
				
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


INT Potato_close(FileSystem* fs, char* path_name, FileOp flag) {
	printf("[Potato_close] enter\n");
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
    // remove the entry if opcount reaches 0
    if(inode_entry->ref == 0) {
        err = removeOpenFileEntry(&(fs->open_file_table), path_name);
        assert(err == Success);
        //CAUTION: if the number of link is 0, no ops here.
    }
    return 0;
}

INT Potato_truncate(FileSystem* fs, char* path_name, size_type newLen) {
    printf("[Potato_truncate] enter\n");
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
            Potato_bfree(fs, cur_inode, file_block_id); 
            file_block_id--;

        }

        while(len > 0 && file_block_id >= 0) {
        	if(len < BLOCK_SIZE) {
        		len = 0;
        	} else {
        		Potato_bfree(fs, cur_inode, file_block_id);
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



//update mod/access time of a file
//1. resolve path
//1. check permission (*)
//2. update inode cache (*)
//3. sync inode (*)
//4. load inode
//5. write inode
INT Potato_utimens(FileSystem *fs, char *path, struct timespec tv[2]) 
{
	
	//INT curINodeID = l2_namei(fs, path);
	size_type curINodeID;
	ErrorCode err_namei = Potato_namei(fs, path, &curINodeID);
	assert (err_namei == Success);
	
	if (curINodeID < 0) {
		return curINodeID;
	}
	
    //readINode(fs, curINodeID, &curINode);
    Inode curINode;
    if(getInode(fs, &curINodeID, &curINode) != Success){
    	printf("[utimens] Error: failed to get Inode!\n");
    	return -1;
    }
	
	curINode.fileModifiedTime = tv[0].tv_sec;
	curINode.fileAccessTime = tv[1].tv_sec;
	
	//writeINode(fs, curINodeID, &curINode);
    ErrorCode err_putInode = putInode(fs, &curINodeID, &curINode);
    if (err_putInode != Success){
       	printf("[utimens] Error: failed to put Inode!\n");
      	return -1;
    }	
	
	return 0;
}



