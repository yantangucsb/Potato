/*
 * Test initialization of potato
 * By Yan
 */

#include <stdio.h>
#include "FileSystem.h"

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
    return 0;
}
