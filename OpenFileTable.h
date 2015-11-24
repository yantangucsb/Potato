/*
 * Data structure for open files
 * By Yan
 */

#include "Parameters.h"
#include "Inode.h"

typedef struct InodeEntry InodeEntry;
struct InodeEntry{
    size_type id;
    size_type ref;
    Inode inode;
    InodeEntry* next;
};

typedef struct InodeTable {
    size_type nInodes;
    InodeEntry* head;
} InodeTable;

typedef enum FileOp {
    READ = 0,
    WRITE = 1,
    READWRITE = 2
} FileOp;

typedef struct OpenFileEntry OpenFileEntry;
struct OpenFileEntry{
    char filePath[FILE_PATH_LENGTH];
    FileOp fileOp;
    size_type ref;
    InodeEntry* inodeEntry;
    OpenFileEntry* next;
};

typedef struct OpenFileTable{
    size_type nOpenFiles;
    OpenFileEntry* head;
} OpenFileTable;

ErrorCode initOpenFileTable(OpenFileTable* open_file_table);

ErrorCode initInodeTable(InodeTable* inode_table);

ErrorCode getOpenFileEntry(OpenFileTable* open_file_table, char* path_name, OpenFileEntry* entry);

ErrorCode addOpenFileEntry(OpenFileTable* open_file_table, char* path_name, FileOp file_op, InodeEntry* inode_entry);

ErrorCode getInodeEntry(InodeTable* inode_table, size_type inode_id, InodeEntry* inode_entry);

/*
 *  inode is addr of a inode, inode_entry is the addr of the entry inserted
 */
ErrorCode addInodeEntry(InodeTable* inode_table, size_type inode_id, Inode* inode, InodeEntry* inode_entry);
