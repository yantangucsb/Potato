/*
 * This file defines operations on data block
 * By: Yan
 */
#include "FileSystem.h"

ErrorCode allocBlock(FileSystem* fs, size_type* blockId);
ErrorCode freeBlock(FileSystem* fs, size_type* blockId);
ErrorCode getBlock(FileSystem* fs, size_type* blockId);
ErrorCode putBlock(FileSystem* fs, size_type* blockId);
void PrintInfo(FileSystem* fs, size_type* blockId);
