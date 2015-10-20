/*
 * Test initialization of potato
 * By Yan
 */

#include <stdio.h>
#include "FileSystem.h"

int main(int argc, char *argv[]){
    FileSystem fs;
    size_type size = 1024*1024;
    initFS(size, 20, &fs);
    printDisk(&(fs.disk_emulator), 0);
    printf("size of Inode: %lu\n", sizeof(Inode));
    printf("size of SuperBlock: %lu\n", sizeof(SuperBlock));
    return 0;
}
