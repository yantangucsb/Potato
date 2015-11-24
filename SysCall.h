#include "Parameters.h"
#include "FileSystem.h"
#include "InodeAccess.h"
#include "sys/stat.h"

ErrorCode bmap(FileSystem* fs, Inode* inode, size_type* offset, size_type* block_no, size_type* block_offset);

ErrorCode namei(FileSystem* fs, char* path_name, size_type* inode_id);

/*
 * open file
 * @flag: type of open, reading or writing, @modes give file permissions if the file is being created
 * return an integer as file descripter
 */
INT open(char* path_name, FileOp flag, mode_t modes);

