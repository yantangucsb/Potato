/*
 * Set any global parameters here
 * Modified by Peng
 * +FreeInodeListFull
 */

#pragma once

#include<stdint.h>

typedef long size_type;
typedef uint32_t addr_type;
typedef uint8_t BYTE;

typedef enum {
    Success = 0,
    DiskFull = 1,
    OutOfBound = 2,
    FreeInodeListFull = 3,
    Err_GetInode = 4,
    Err_PutInode = 5,
    Err_InitInode = 6,
    Err_GarbCollectInode = 7,
    Err_GetBlock = 8,
    Err_PutBlock = 9,
    Err_InodeFull = 10,
    NoFreeDataBlock = 11,
    NotDirectory = 13,      //for namei
    InodeNotExist = 14
} ErrorCode;

typedef enum {
    Regular = 0,
    Directory = 1//,
//    CharactorDeviceFile = 2,
//    BlockDeviceFile = 3,
//    LocalSocketFile = 4,
//    Pipes = 5,
//    SymbolicLink = 6
} FileType;


#define BLOCK_SIZE 1024

//Super block parameters
#define FREE_INODE_NUM 100
#define SUPER_BLOCK_OFFSET 0

//free list parameters
#define FREE_BLOCK_ARRAY_SIZE BLOCK_SIZE/sizeof(size_type)-1

#define FILE_OWNER_LENGTH 14
#define FILE_NAME_LENGTH 14
#define PERMISSION_CAT_NUM 3
#define DIRECT_BLOCK_NUM 10

#define PERCENT 20

//Inode Time
#define TIME_LENGTH 32

#define ROOT_INODE_ID 0
#define FILE_PATH_LENGTH 512
