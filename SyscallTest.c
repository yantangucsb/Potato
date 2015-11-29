#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "SysCall.h"

int main(){

    FileSystem fs;
    loadFS(&fs);
    printDisk(&(fs.disk_emulator), 1);
    printFileSystem(&fs);

    Inode inode;
    size_type id = 0;
    getInode(&fs, &id, &inode);
    printf("root links %ld\n", inode.numOfLinks);
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
