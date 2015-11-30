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

    printf("DirEntry size: %ld\n", sizeof(DirEntry));
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
    printf("root inodeId: %ld\n root links: %ld, root size: %ld, root permission: %d\n", root_stat.st_ino, root_stat.st_nlink, root_stat.st_size, root_stat.st_mode);
    
    inode_id = Potato_mknod(&fs, "/hello", 0, 0);
    Inode hello_inode;
    getInode(&fs, &inode_id, &hello_inode);
    printf("hello inode_od: %ld, filetype: %d, root permission: %d\n", inode_id, hello_inode.fileType, root_stat.st_mode);
    struct stat hello_stat;
    Potato_getattr(&fs, "/hello", &hello_stat);
    printf("hello inodeId: %ld\n hello links: %ld, hello size: %ld, hello permission: %d\n", hello_stat.st_ino, hello_stat.st_nlink, hello_stat.st_size, hello_stat.st_mode);

//    DirEntry dir_entry;
//    Potato_readdir(&fs, "/", 0, &dir_entry);
//    printf("dir entry key: %s, inode id: %ld\n", dir_entry.key, dir_entry.inodeId);
//    Potato_getattr(&fs, "/hello", &root_stat);
    inode_id = Potato_mkdir(&fs, "/dir", 0, 0);
    Inode dir_inode;
    getInode(&fs, &inode_id, &dir_inode);
    printf("dir inode_id: %ld, filetype: %d\n", inode_id, dir_inode.fileType);

    struct stat dir_stat;
    Potato_getattr(&fs, "/dir", &dir_stat);
    printf("dir inodeId: %ld\n dir links: %ld, dir size: %ld, dir permission: %d\n", dir_stat.st_ino, dir_stat.st_nlink, dir_stat.st_size, dir_stat.st_mode);
//    Potato_namei(&fs, "/dir", &inode_id);
    Potato_open(&fs, "/hello", WRITE);
    char str[10] = "happy";
    Potato_write(&fs, "/hello", 0, str, sizeof(str));
    printOpenFileTable(&(fs.open_file_table));
    printInodeTable(&(fs.inode_table));
    return 1;
}
