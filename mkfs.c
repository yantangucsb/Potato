#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "FileSystem.h"
#include "DataBlockAccess.h"
#include "InodeAccess.h"

int main(){
    FileSystem fs;
    size_type size = 0x7FFFFFF;
    printf("system size set to: %ld\n", size);
    initFS(size, 20, &fs);
    loadFS(&fs);
    printFileSystem(&fs);
//    printDisk(&(fs.disk_emulator), 0);

    Inode inode;
    size_type inode_id = ROOT_INODE_ID;
    if(getInode(&fs, &inode_id, &inode)!=Success){
        printf("[Potato mount] Error: root directory not exist.\n");
        //return Err_GetInode;
        return -1;
    }

    loadFS(&fs);
    //allocate a block to root directory
    size_type block_id;
    allocBlock(&fs, &block_id);
    inode.directBlock[0] = block_id;
    inode.fileType = Directory;
    inode.used = true;
    inode.Permission = S_IFDIR | 0755;

    DirEntry dir_entry[2];
    strcpy(dir_entry[0].key, ".");
    dir_entry[0].inodeId = ROOT_INODE_ID;
    strcpy(dir_entry[1].key, "..");
    dir_entry[1].inodeId = ROOT_INODE_ID;

    BYTE* buf = malloc(BLOCK_SIZE);
    memcpy(buf, dir_entry, sizeof(dir_entry));
    put(&fs, block_id + fs.super_block.firstDataBlockId, buf);
    
    inode.fileSize = sizeof(dir_entry);
    
    //for . and ..
    inode.numOfLinks = 2;
    putInode(&fs, &inode_id, &inode);

    //put free list buf into disk
    put(&fs, fs.super_block.pDataFreeListHead + fs.super_block.firstDataBlockId, &(fs.dataBlockFreeListHeadBuf));
    put(&fs, fs.super_block.pDataFreeListTail + fs.super_block.firstDataBlockId, &(fs.dataBlockFreeListTailBuf));

    
    SuperBlockonDisk super_block_on_disk;
    mapSuperBlockonDisk(&(fs.super_block), &(super_block_on_disk));
    memcpy(buf, &super_block_on_disk, sizeof(SuperBlockonDisk));
    put(&fs, SUPER_BLOCK_OFFSET, buf);
    //set root acess time

    free(buf);
    closefs(&fs);

    FileSystem new_fs;
    loadFS(&new_fs);
    printFileSystem(&new_fs);
    closefs(&fs);   
    return 0;
}
