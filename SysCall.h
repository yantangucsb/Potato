#include "Parameters.h"
#include "FileSystem.h"
#include "InodeAccess.h"
#include "DataBlockAccess.h"
#include "sys/stat.h"
#include "sys/types.h"



//by yan
ErrorCode Potato_bmap(FileSystem* fs, Inode* inode, size_type* offset, size_type* block_no, size_type* block_offset);
INT Potato_namei(FileSystem* fs, char* path_name, size_type* inode_id);
/*
 * open file
 * @flag: type of open, reading or writing, @modes give file permissions if the file is being created
 * return an integer as file descripter
 */
INT Potato_open(FileSystem* fs, char* path_name, FileOp flag);


//by peng
// mounts a filesystem from a device
INT Potato_mount(FileSystem* fs);
// unmounts a filesystem into a device
INT Potato_unmount(FileSystem* fs);
// makes a new file
INT Potato_mknod(FileSystem* fs, char* path, uid_t uid, gid_t gid);
// deletes a file or directory
INT Potato_unlink(FileSystem* fs, char* path);
// makes a new directory
INT Potato_mkdir(FileSystem* fs, char* path, uid_t uid, gid_t gid);
// reads directory contents
INT Potato_readdir(FileSystem* fs, char* path, LONG offset, DirEntry* curEntry);
//change mode
INT Potato_chmod(FileSystem* fs, char* path, uint32_t set_permission);
// getattr
INT Potato_getattr(FileSystem* fs, char *path, struct stat *stbuf);
//1. resolve path and read inode
//2. check uid/gid
//3. set uid/gid and write inode
INT Potato_chown(FileSystem *fs, char *path, uid_t uid, gid_t gid);



//by marco
// read file from offset for numBytes
// 1/2/3. look in open file table for entry
// 4. modify modtime
// 5. call readINodeData on current INode, offset to the buf for numBytes
// 6. write back inode
INT Potato_read(FileSystem* fs, char* path_name, size_type offset, BYTE* buf, size_type numBytes);

//write file from offset for numBytes
//1. resolve path
//2. get INodeTable Entry
//3. load inode
//4. call writeINodeData
//5. modify inode if necessary
INT Potato_write(FileSystem* fs, char* path_name, size_type offset, BYTE* buf, size_type numBytes);

INT Potato_rename(FileSystem* fs, char* path_name, char* new_path_name);

INT Potato_close(FileSystem* fs, char* path_name, FileOp flag);

// truncate a file
INT Potato_truncate(FileSystem* fs, char* path_name, size_type newLen);

INT Potato_utimens(FileSystem *fs, char *path, struct timespec tv[2]);

