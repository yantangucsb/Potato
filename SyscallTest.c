#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "SysCall.h"

int main(){

    FileSystem fs;
    loadFS(&fs);
    printDisk(&(fs.disk_emulator), 0);
    printFileSystem(&fs);
/*
    SuperBlockonDisk sb_ondisk;
    BYTE* buf = malloc(BLOCK_SIZE);
    get(&fs, 0, buf);
    memcpy(&sb_ondisk, buf, sizeof(SuperBlockonDisk));
    mapDisk2SuperBlockinMem(&sb_ondisk, &(fs.super_block));
    printFileSystem(&fs);
  */  

    size_type inode_id;
    if(Potato_namei(&fs, "/", &inode_id) != Success){
        printf("[Layer1Test] Potato_namei failed for path /\n");
    }
    printf("[Layer1Test] root directory inode id: %ld\n", inode_id);

    struct stat root_stat;
    Potato_getattr(&fs, "/", &root_stat);
    printf("root inodeId: %ld\n root links: %ld, root size: %ld, root permission: %u\n", root_stat.st_ino, root_stat.st_nlink, root_stat.st_size, root_stat.st_mode);
    return 1;

}
