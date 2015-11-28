#include "Parameters.h"
#include "FileSystem.h"
#include "InodeAccess.h"
#include "sys/stat.h"

ErrorCode Potato_bmap(FileSystem* fs, Inode* inode, size_type* offset, size_type* block_no, size_type* block_offset);

ErrorCode Potato_namei(FileSystem* fs, char* path_name, size_type* inode_id);

/*
 * open file
 * @flag: type of open, reading or writing, @modes give file permissions if the file is being created
 * return an integer as file descripter
 */
INT Potato_open(FileSystem* fs, char* path_name, FileOp flag, mode_t modes);

INT Potato_read(FileSystem* fs, char* path_name, size_type offset, BYTE* buf, size_type numBytes);

INT Potato_write(FileSystem* fs, char* path_name, size_type offset, BYTE* buf, size_type numBytes);

INT Potato_truncate(FileSystem* fs, char* path_name, size_type newLen);

INT Potato_close(FileSystem* fs, char* path_name, FileOp flag);

INT Potato_rename(FileSystem* fs, char* path_name, char* new_path_name);

