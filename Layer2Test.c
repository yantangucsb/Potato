/**
 * Tests Layer 2 directories
 * by Jon
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Directories.h"
#define NUM_SUB_DIR 2

void printMenu();
INT mk_tree(FileSystem *fs, char* path, INT nINodes);

int main(int args, char* argv[])
{
    srand(time(NULL));
    
    if (args < 3) {
        printf("Usage: TestMain FS_NUM_DATA_BLOCKS FS_NUM_INODES\n");
        exit(1);
    }

    UINT nDBlks = atoi(argv[1]);
    UINT nINodes = atoi(argv[2]);
    
    FileSystem fs;
    printf("Initializing file system with initfs...\n");
    UINT succ = l2_initfs(nDBlks, nINodes, &fs);
    if(succ == 0) {
        printf("initfs succeeded with filesystem size: %d\n", fs.nBytes);
    }
    else {
        printf("Error: initfs failed with error code: %d\n", succ);
    }
    
    BOOL quit = false;
    char command[1024];
    char path[1024];
    char buf[1024];
//    char namelist[MAX_FILE_NUM_IN_DIR][FILE_NAME_LENGTH];
    UINT offset;
    UINT len;

    // root directory testing 
    assert(l2_mkdir(&fs, "/", 0, 0) == -1);
    assert(fs.superblock.nFreeINodes == nINodes - 1);
    
    INode rootInode;
    if(readINode(&fs, fs.superblock.rootINodeID, &rootInode) == -1){
        fprintf(stderr, "error: read inode %d from disk\n", fs.superblock.rootINodeID);
        return -1;
    }
    assert(rootInode._in_filesize == 2*sizeof(DirEntry));

    // create nINodes -1 subdirectories in root
    for (INT i = 0; i < nINodes - 1; i ++) {
        sprintf(path, "/%d", i);
        if(l2_mkdir(&fs, path, 0, 0) == -1) {
            fprintf(stderr, "error: fail to create directory for %s\n", path);
            return -1;
        }
        //printf("%s\n", path);
        printf("remaining free inodes = %d\n", fs.superblock.nFreeINodes);
    }
    assert(fs.superblock.nFreeINodes == 0);
    if(readINode(&fs, fs.superblock.rootINodeID, &rootInode) == -1){
        fprintf(stderr, "error: read inode %d from disk\n", fs.superblock.rootINodeID);
        return -1;
    }
    printf("rootINode directory entry num = %d\n", rootInode._in_filesize/(sizeof(DirEntry)));
    assert(rootInode._in_filesize == (nINodes + 1)*sizeof(DirEntry));

    // test creating a new directory when there is no more inodes
    sprintf(path, "/%d", nINodes - 1);
//    assert(l2_mkdir(&fs, path, 0, 0) == -1); 

    // unlink all the directories created
    for (INT i = 0; i < nINodes - 1; i ++) {
        sprintf(path, "/%d", i);
        if(l2_unlink(&fs, path) == -1) {
            fprintf(stderr, "error: fail to unlink directory %s\n", path);
            return -1;
        }
        printf("remaining free inodes = %d\n", fs.superblock.nFreeINodes);
    }

    assert(fs.superblock.nFreeINodes == nINodes - 1);
    if(readINode(&fs, fs.superblock.rootINodeID, &rootInode) == -1){
        fprintf(stderr, "error: read inode %d from disk\n", fs.superblock.rootINodeID);
        return -1;
    }
   
    // create nINodes - 1 files in root 
    for (INT i = 0; i < nINodes - 1; i ++) {
        sprintf(path, "/%d", i);
        if(l2_mknod(&fs, path, 0, 0) == -1) {
            fprintf(stderr, "error: fail to create file %s\n", path);
            return -1;
        }
        //printf("%s\n", path);
        printf("remaining free inodes = %d\n", fs.superblock.nFreeINodes);
    }
    assert(fs.superblock.nFreeINodes == 0);
    if(readINode(&fs, fs.superblock.rootINodeID, &rootInode) == -1){
        fprintf(stderr, "error: read inode %d from disk\n", fs.superblock.rootINodeID);
        return -1;
    }
    assert(rootInode._in_filesize == (nINodes + 1)*sizeof(DirEntry));

    // test creating a new file when there is no more inodes
    sprintf(path, "/%d", nINodes - 1);
//    assert(l2_mknod(&fs, path, 0, 0) == -1); 

    // unlink all files created
    for (INT i = 0; i < nINodes - 1; i ++) {
        sprintf(path, "/%d", i);
        if(l2_unlink(&fs, path) == -1) {
            fprintf(stderr, "error: fail to unlink directory %s\n", path);
            return -1;
        }
        printf("remaining free inodes = %d\n", fs.superblock.nFreeINodes);
    }

    assert(fs.superblock.nFreeINodes == nINodes - 1);
    if(readINode(&fs, fs.superblock.rootINodeID, &rootInode) == -1){
        fprintf(stderr, "error: read inode %d from disk\n", fs.superblock.rootINodeID);
        return -1;
    }
   
    // recursively create subdirectories and files  
    if(mk_tree(&fs, "/", nINodes) == -1) {
        fprintf(stderr, "error: fail to create a tree of directories and files\n");
 
    }
    printf("after creating the tree, remaining inodes = %d\n", fs.superblock.nFreeINodes);

    //TODO: unlink all the directories and files (bottom up vs top-down)

    return 0;
}

// recursively create subdirectories and files under the directory "path"
INT mk_tree(FileSystem *fs, char* path, INT nINodes) {
    printf("enter mk_tree\n");
    char dir_path[1024];
    char file_path[1024];
    static INT i = 0; // level
    i ++;

    if(fs->superblock.nFreeINodes >= 2) {
        //for (INT i = 0; i < NUM_SUB_DIR; i ++) {
            strcpy(dir_path, path);
            strcpy(file_path, path);
            
            char dir_name[1024];
            char file_name[1024];

            if(strcmp(dir_path, "/") == 0) {
                sprintf(dir_name, "dir_%d", i);
                sprintf(file_name, "file_%d", i);
            }
            else {
                sprintf(dir_name, "/dir_%d", i);
                sprintf(file_name, "/file_%d", i);
            }

            strcat(dir_path, dir_name);
            strcat(file_path, file_name);

            printf("sub dir name = %s\n", dir_path);
            if(l2_mkdir(fs, dir_path, 0, 0) == -1) {
                fprintf(stderr, "error: fail to create directory for %s\n", dir_path);
                return -1;
            }
            
            printf("sub file name = %s\n", file_path);
            if(l2_mknod(fs, file_path, 0, 0) == -1) {
                fprintf(stderr, "error: fail to create file for %s\n", file_path);
                return -1;
            }
            
            mk_tree(fs, dir_path, nINodes);
            
        //}
    }
    else {
        assert(i == (nINodes - 1 )/ 2 + 1);
    }

    return 0;
}
