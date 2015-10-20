/*
 * Set any global parameters here
 */

#pragma once

#include<stdint.h>

typedef long size_type;
typedef uint32_t addr_type;
typedef uint8_t BYTE;

typedef enum {
    Success = 0,
    DiskFull = 1,
    OutOfBound = 2
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


#define BLOCK_SIZE 512

//Super block parameters
#define FREE_INODE_NUM 10
#define SUPER_BLOCK_OFFSET 0

//free list parameters
#define FREE_BLOCK_ARRAY_SIZE BLOCK_SIZE/sizeof(size_type)-1

#define FILE_OWNER_LENGTH 14
#define PERMISSION_CAT_NUM 3
#define DIRECT_BLOCK_NUM 10
