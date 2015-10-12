/*
 * Set any global parameters here
 */

#include<stdint.h>

typedef long size_type;
typedef uint32_t addr_type;

typedef enum {
    Success = 0,
    DiskFull = 1,
    OutOfBound = 2
} ErrorCode;

#define BLOCK_NUM 10
#define BLOCK_SIZE 256
#define DISK_SIZE BLOCK_NUM*BLOCK_SIZE

#define INODE_NUM 10

#define FILE_OWNER_LENGTH 50
#define PERMISSION_CAT_NUM 3
#define DIRECT_BLOCK_NUM 10
