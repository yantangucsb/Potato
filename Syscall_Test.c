/**
 * Tests Layer 2 directories
 * by Jon
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "SysCall.h"

void printMenu();

int main(int args, char* argv[])
{
    srand(time(NULL));
    
    if (args < 3) {
        printf("Usage: Syscall_Test FS_SIZE FS_PERCENT\n");
        exit(1);
    }

    size_type fs_size = atoi(argv[1]);
    size_type fs_percent = atoi(argv[2]);
    
    FileSystem fs;
    printf("Initializing file system with initfs...\n");
    
    //UINT succ = l2_initfs(nDBlks, nINodes, &fs);
    //ErrorCode initFS(size_type size, size_type percen, FileSystem* fs)
    ErrorCode err_initFS = initFS(fs_size, fs_percent, &fs);
    if(err_initFS == Success) {
    	printf("initfs succeeded with filesystem size: %ld\n", fs.super_block.systemSize);
    }
    else{
    	printf("Error: initfs failed with error code: %d\n", err_initFS);
    }
    
    BOOL quit = false;
    char command[1024];
    char path[1024];
    char new_path[1024];
    uint32_t flags;
    char buf[1024];
    //char namelist[MAX_FILE_NUM_IN_DIR][FILE_NAME_LENGTH];
    uint32_t offset;
    uint32_t len;
    
    while(!quit) {
        printMenu();
        printf("\nEnter a command: ");
        scanf("%s", command);

        memset(buf, 0, sizeof(buf));        

        if(strcmp(command, "mkdir") == 0) {
            //printf("Enter directory path: ");
            scanf("%s", path);
            
            Potato_mkdir(&fs, path, 0, 0);
        }
        else if(strcmp(command, "mknod") == 0) {
            //printf("Enter file path: ");
            scanf("%s", path);
            
            Potato_mknod(&fs, path, 0, 0);
        }
        else if (strcmp(command, "truncate") == 0) {
            //printf("Enter file path: ");
            scanf("%s", path);
            //printf("Enter file offset: ");
            scanf("%d", &offset);
            
            Potato_truncate(&fs, path, offset);
        }
        else if(strcmp(command, "readdir") == 0) {
            //printf("Enter file path: ");
            scanf("%s", path);
            
            Potato_readdir(&fs, path, namelist);
        }
        else if(strcmp(command, "rename") == 0) {
            //printf("Enter file path: ");
            scanf("%s", path);
            //printf("Enter new file path: ");
            scanf("%s", new_path);
            
            Potato_rename(&fs, path, new_path);
        }
        else if(strcmp(command, "unlink") == 0) {
            //printf("Enter file path: ");
            scanf("%s", path);
            
            Potato_unlink(&fs, path);
        }
        else if(strcmp(command, "open") == 0) {
            //printf("Enter file path: ");
            scanf("%s", path);
            //printf("Enter open flags: ");
            scanf("%d", &flags);
            
            Potato_open(&fs, path, flags);
        }
        else if(strcmp(command, "close") == 0) {
            //printf("Enter file path: ");
            scanf("%s", path);
            //printf("Enter close flags: ");
            scanf("%d", &flags);
            
            Potato_close(&fs, path, flags);
        }
        else if(strcmp(command, "read") == 0) {
            //printf("Enter file path: ");
            scanf("%s", path);
            //printf("Enter file offset: ");
            scanf("%d", &offset);
            //printf("Enter read length: ");
            scanf("%d", &len);
            
            Potato_read(&fs, path, offset, buf, len);
            printf("Read data: %s", buf);
        }
        else if(strcmp(command, "write") == 0) {
            //printf("Enter file path: ");
            scanf("%s", path);
            //printf("Enter file offset: ");
            scanf("%d", &offset);
            //printf("Enter data to write (text): ");
            scanf("%s", buf);
            
            len = strlen(buf);
            Potato_write(&fs, path, offset, buf, len);
        }
        else if(strcmp(command, "corestats") == 0) {
            printf("\nSuperblock:\n");
            printSuperBlock(&fs.superblock);
            printf("\nFree inode cache:\n");
            printFreeINodeCache(&fs.superblock);
            printf("\nFree dblk cache:\n");
            printFreeDBlkCache(&fs.superblock);
            printf("\nDBlkCache:\n");
            printDBlkCache(&fs.dCache);
            printf("\nOpen file table:\n");
            printOpenFileTable(&fs.openFileTable);
            printf("\nINode table:\n");
            printINodeTable(&fs.inodeTable);
            printf("\nINode cache:\n");
            printINodeCache(&fs.inodeCache);
        }
        else if(strcmp(command, "diskstats") == 0) {
            printf("\nSuperblock:\n");
            printSuperBlock(&fs.superblock);
            printf("\nINodes:\n");
            printINodes(&fs);
            printf("\nData blocks:\n");
            printDBlks(&fs);
            printf("Stats only available in DEBUG mode!\n");
        }
        else if(strcmp(command, "quit") == 0) {
            quit = true;
        }
        else {
            printf("Invalid command entered!\n");
        }
    }
    return 0;
}

void printMenu() {
    printf("\n");
    printf("====================\n");
    printf("File system commands\n");
    printf("--------------------\n");
    printf("IMPORTANT: please provide an absolute path!\n");
    printf(" mkdir /path/to/directory\n");
    printf(" mknod /path/to/file\n");
    printf(" rename /path/one /path/two\n");
    printf(" unlink /path/to/file_or_dir\n");
    printf(" open /path/to/file_or_dir FLAGS\n");
    printf(" close /path/to/file_or_dir FLAGS\n");
    printf(" read /path/to/file_or_dir OFFSET LEN\n");
    printf(" write /path/to/file_or_dir OFFSET data\n");
    printf("--------------------\n");
    printf(" corestats\n");
    printf(" diskstats\n");
    printf(" quit\n");
    printf("====================\n");
}
