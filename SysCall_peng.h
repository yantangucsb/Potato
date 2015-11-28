/*
 * This file implements mount, unmount, mknod, unlink, mkdir, readdir, chmod
 * By: Peng
 */
#include "Directory.h"
#include "FileSystem.h"
#include "sys/stat.h"
#include "sys/types.h"

// mounts a filesystem from a device
//ok
ErrorCode Potato_mount(FileSystem* fs);

// unmounts a filesystem into a device
ErrorCode Potato_unmount(FileSystem* fs);

// makes a new file
ErrorCode Potato_mknod(FileSystem* fs, char* path, uid_t uid, gid_t gid, size_type* inodeId);

// deletes a file or directory
ErrorCode Potato_unlink(FileSystem* fs, char* path);

// makes a new directory
INT Potato_mkdir(FileSystem* fs, char* path, uid_t uid, gid_t gid);

// reads directory contents
INT Potato_readdir(FileSystem* fs, char* path, LONG offset, DirEntry* curEntry);

//change mode
INT Potato_chmod(FileSystem* fs, char* path, mode_t mode);


