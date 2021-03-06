/*
 * Interface of FileSystem
 * BY Yan
 */
#pragma once
#include "SuperBlock.h"
#include "FreeListNode.h"
#include "DiskEmulator.h"
#include "OpenFileTable.h"
#include "Directory.h"

typedef struct FileSystem{
    SuperBlock super_block;

    //in memory buffer (size of BLOCK_SIZE) for head and tail of data block free list
    FreeListNode dataBlockFreeListHeadBuf;
    FreeListNode dataBlockFreeListTailBuf;
    
    OpenFileTable open_file_table;

    InodeTable inode_table;

    DiskEmulator disk_emulator;

} FileSystem;

ErrorCode setDataBlockFreeList(FileSystem* fs);

ErrorCode put(FileSystem* fs, size_type block_no, void* buffer);

ErrorCode get(FileSystem* fs, size_type block_no, void* buffer);

ErrorCode initFS(size_type size, size_type percen, FileSystem* fs);

ErrorCode loadFS(FileSystem* fs);

ErrorCode readSuperBlock(FileSystem* fs);

ErrorCode closefs(FileSystem* fs);

void printFileSystem(FileSystem* fs);

