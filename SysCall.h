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

// makes a new directory
ErrorCode Potato_mkdir(FileSystem* fs, char* path, uid_t uid, gid_t gid, size_type* inodeId);

// reads directory contents
ErrorCode Potato_readdir(FileSystem* fs, char* path, LONG offset, DirEntry* curEntry, size_type* inodeId);

//change mode
//TODO
//no mode input
ErrorCode Potato_chmod(FileSystem* fs, char* path, size_type* inodeId);




//by marco
INT Potato_read(FileSystem* fs, char* path_name, size_type offset, BYTE* buf, size_type numBytes);

INT Potato_write(FileSystem* fs, char* path_name, size_type offset, BYTE* buf, size_type numBytes);

INT Potato_close(FileSystem* fs, char* path_name, FileOp flag);

INT Potato_rename(FileSystem* fs, char* path_name, char* new_path_name);

/*
 * open file
 * @flag: type of open, reading or writing, @modes give file permissions if the file is being created
 * return an integer as file descripter
 */
INT Potato_open(FileSystem* fs, char* path_name, FileOp flag, mode_t modes);

