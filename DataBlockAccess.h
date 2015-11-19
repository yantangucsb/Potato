/*
 * This file defines operations on data block
 * By: Yan
 */
#include "FileSystem.h"

ErrorCode allocBlock(FileSystem* fs, size_type* blockId);
ErrorCode freeBlock(FileSystem* fs, size_type* blockId);
