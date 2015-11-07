/*
 * This file defines allocInode, garbcollectInode, freeInode, InitInode, getInode.
 * By: Peng
 */
#include "FileSystem.h"
ErrorCode allocInode(FileSystem* fs, size_type* inodeId, Inode* inode);
ErrorCode garbcollectInode(FileSystem* fs);
ErrorCode garbcollectInode(FileSystem* fs);
ErrorCode freeInode(FileSystem* fs, size_type* inodeId);
ErrorCode InitInode(Inode* inode);
ErrorCode getInode(FileSystem* fs, size_type* inodeId, Inode* inode);
ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode);
void PrintInfo(SuperBlock* super, size_type* inodeId);
