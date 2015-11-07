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
//    printf("size of Inode: %lu\n", sizeof(Inode));
//    printf("size of SuperBlock: %lu\n", sizeof(SuperBlock));
    readSuperBlock(&fs);
    printFileSystem(&fs);
    size_type inodeId;
    Inode inode;
    ErrorCode err;
    int i;
    int Current_Number_Of_Inodes=104856;
    for (i=0;i<101;i++)
    	allocInode(&fs, &inodeId, &inode);
    return 0;
}
