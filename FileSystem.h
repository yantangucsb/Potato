/*
 * Interface of FileSystem
 * BY Yan
 */

#include "Parameters.h"
#include "Inode.h"

typedef struct FileSystem{
    SuperBlock super_block;

    //in memory buffer (size of BLOCK_SIZE) for head and tail of data block free list
    addr_type dataBlockFreeListHeadBuf;
    addr_type dataBlockFreeListTailBuf;
    
    DiskEmulator* disk_emulator;

} FileSystem;

ErrorCode initFS(DiskEmulator *disk_emulator, long size, int percen);

