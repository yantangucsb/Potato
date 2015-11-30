/*
 * This file defines operations on data block
 * By: Yan
 */
#include "FileSystem.h"

ErrorCode allocBlock(FileSystem* fs, size_type* blockId);
ErrorCode freeBlock(FileSystem* fs, size_type* blockId);
//write buf to data block with offset
ErrorCode writeDataBlock(FileSystem* fs, size_type block_no, BYTE* buf, size_type start, size_type size);
//read buf from datablock with offset
ErrorCode readDataBlock(FileSystem* fs, size_type block_no, BYTE* buf, size_type start, size_type size);
