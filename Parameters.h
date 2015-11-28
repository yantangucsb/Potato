/*
 * Set any global parameters here
 * Modified by Peng
 * +FreeInodeListFull
 */

#pragma once

#include<stdint.h>

typedef long size_type;
typedef int64_t LONG;
typedef uint32_t addr_type;
typedef uint8_t BYTE;
typedef int32_t INT;

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
    NotDirectory = 12,      //for namei
    InodeNotExist = 13,
    Err_OpenDisk = 14, //for mount
    Err_FailReadSuperblk = 15, //for mount
    Err_mapDisk2SuperBlockinMem = 16, //for mount
    Err_mapSuperBlockonDisk = 17, //for mount
    Err_initOpenFileTable = 18, //for mount
    Err_initInodeTable = 19, //for mount
    Err_destroyDisk = 20, //for closefs
    Err_mknod = 21, //for mknod
    Err_unlink = 22, //for unlink
    
} ErrorCode;

typedef enum {
    Read = 0,
    Write = 1,
    Execute = 2
} FilePermType;

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

//should be checked together with enum FilePermType
#define PERMISSION_CAT_NUM 3
#define DIRECT_BLOCK_NUM 10
#define S_DIRECT_BLOCK_NUM 1
#define D_DIRECT_BLOCK_NUM 1
#define T_DIRECT_BLOCK_NUM 1


#define PERCENT 20

//Inode Time
#define TIME_LENGTH 32

#define ROOT_INODE_ID 0
#define FILE_PATH_LENGTH 512 //maximum length of the path

//disk partition path
#define DISK_PATH "diskFile"

#define MAX_FILE_SIZE (BLOCK_SIZE * DIRECT_BLOCK_NUM + BLOCK_SIZE / sizeof(size_type) * BLOCK_SIZE + BLOCK_SIZE / sizeof(size_type) * BLOCK_SIZE / sizeof(size_type) * BLOCK_SIZE + BLOCK_SIZE / sizeof(size_type) * BLOCK_SIZE / sizeof(size_type) * BLOCK_SIZE / sizeof(size_type) * BLOCK_SIZE)
#define MAX_FILE_BLKS (MAX_FILE_SIZE / BLOCK_SIZE) //max number of data blocks allocatable per file
#define MAX_FILE_NUM_IN_DIR (MAX_FILE_SIZE / (FILE_NAME_LENGTH + sizeof(INT))) //maximum number of files in a directory






