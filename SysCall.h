#include "Parameters.h"
#include "FileSystem.h"
#include "InodeAccess.h"

ErrorCode bmap(FileSystem* fs, Inode* inode, size_type* offset, size_type* block_no, size_type* block_offset);

ErrorCode namei(FileSystem* fs, char* path_name, size_type* inode_id, Inode* inode);
