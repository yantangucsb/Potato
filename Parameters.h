/*
 * Set any global parameters here
 */

#include<stdint.h>

typedef long size_type;
typedef uint32_t addr_type;
typedef uint8_t BYTE;

typedef enum {
    Success = 0,
    DiskFull = 1,
    OutOfBound = 2
} ErrorCode;

#define BLOCK_SIZE 256

//Super block parameters
#define FREE_INODE_NUM 10
#define SUPER_BLOCK_OFFSET 0

#define FILE_OWNER_LENGTH 50
#define PERMISSION_CAT_NUM 3
#define DIRECT_BLOCK_NUM 10
