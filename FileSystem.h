/*
 * Interface of FileSystem
 * BY Yan
 */

#include "SuperBlock.h"
#include "FreeListNode.h"
#include "DiskEmulator.h"

typedef struct FileSystem{
    SuperBlock super_block;

    //in memory buffer (size of BLOCK_SIZE) for head and tail of data block free list
    FreeListNode dataBlockFreeListHeadBuf;
    FreeListNode dataBlockFreeListTailBuf;
    
    DiskEmulator disk_emulator;

} FileSystem;

ErrorCode put(FileSystem* fs, size_type block_no, void* buffer);

ErrorCode get(FileSystem* fs, size_type block_no, void* buffer);

ErrorCode initFS(size_type size, size_type percen, FileSystem* fs);

ErrorCode readSuperBlock(FileSystem* fs);

void printFileSystem(FileSystem* fs);
