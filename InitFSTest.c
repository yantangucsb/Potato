/*
 * Test initialization of potato
 * By Yan
 */

#include <stdio.h>
#include "InodeAccess.h"

int main(int argc, char *argv[]){
    FileSystem fs;
    size_type size = 0x7FFFFFF;
    printf("system size set to: %ld\n", size);
    initFS(size, 20, &fs);
//    printDisk(&(fs.disk_emulator), 0);
    printf("size of Inode: %lu\n", sizeof(Inode));
    printf("size of SuperBlock: %lu\n", sizeof(SuperBlock));
    printFileSystem(&fs);
    readSuperBlock(&fs);
    printFileSystem(&fs);

    
    
    
    
    size_type inodeId;
    Inode inode;
    
    int i;
    for (i=0;i<103;i++)
    	allocInode(&fs, &inodeId, &inode);
    printf("[Get Inode] Test is Successful ~(￣▽￣)~\n");
    printf("[Initialize Inode] Test is Successful ~(￣▽￣)~\n");
    printf("[Put Inode] Test is Successful ~(￣▽￣)~\n");
    printf("[Allocate Inode] Test is Successful ~(￣▽￣)~\n");
    printFileSystem(&fs);
    for (inodeId=0;inodeId<103;inodeId++)
    	freeInode(&fs, &inodeId);
    printf("[Free Inode] Test is Successful ~(￣▽￣)~\n");
    
    for (i=0;i<103;i++)
    	allocInode(&fs, &inodeId, &inode);
    printf("[Garbage Collection] Test is Successful ~(￣▽￣)~\n");
    
    
    
    return 0;
}
