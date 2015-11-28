#include "Parameters.h"
#include "FileSystem.h"
#include "InodeAccess.h"
#include "DataBlockAccess.h"
#include "sys/stat.h"
#include "sys/types.h"

ErrorCode Potato_bmap(FileSystem* fs, Inode* inode, size_type* offset, size_type* block_no, size_type* block_offset);

ErrorCode Potato_namei(FileSystem* fs, char* path_name, size_type* inode_id);

// mounts a filesystem from a device
ErrorCode Potato_mount(FileSystem* fs);

// unmounts a filesystem into a device
ErrorCode Potato_unmount(FileSystem* fs);

// makes a new file
ErrorCode Potato_mknod(FileSystem* fs, char* path, uid_t uid, gid_t gid, size_type* inodeId);

// deletes a file or directory
ErrorCode Potato_unlink(FileSystem* fs, char* path, size_type* inodeId);

// deletes a file or directory
ErrorCode Potato_unlink(FileSystem* fs, char* path, size_type* inodeId);
/*
 * open file
 * @flag: type of open, reading or writing, @modes give file permissions if the file is being created
 * return an integer as file descripter
 */
INT Potato_open(FileSystem* fs, char* path_name, FileOp flag, mode_t modes);

