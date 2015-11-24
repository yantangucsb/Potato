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
size_type getFreeListNum (SuperBlock* super);
void PrintInfo(SuperBlock* super, size_type* inodeId);

//Read inode data sections into buf
//Implement this function
ErrorCode readInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size);
