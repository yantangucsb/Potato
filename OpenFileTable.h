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
    READWRITE = 2,
    TRUNCATE = 3,
    APPEND = 4
} FileOp;

typedef struct OpenFileEntry OpenFileEntry;
struct OpenFileEntry{
    char filePath[FILE_PATH_LENGTH];
    FileOp fileOp;
    size_type ref;
    size_type offset;
    InodeEntry* inodeEntry;
    OpenFileEntry* next;
};

typedef struct OpenFileTable{
    size_type nOpenFiles;
    OpenFileEntry* head;
} OpenFileTable;

ErrorCode initOpenFileTable(OpenFileTable* open_file_table);

ErrorCode initInodeTable(InodeTable* inode_table);

OpenFileEntry* getOpenFileEntry(OpenFileTable* open_file_table, char* path_name);

ErrorCode addOpenFileEntry(OpenFileTable* open_file_table, char* path_name, FileOp file_op, InodeEntry** inode_entry);

InodeEntry* getInodeEntry(InodeTable* inode_table, size_type inode_id);

/*
 *  inode is addr of a inode, inode_entry is the addr of the entry inserted
 */
ErrorCode addInodeEntry(InodeTable* inode_table, size_type inode_id, Inode* inode, InodeEntry** inode_entry);

ErrorCode hasINodeEntry(InodeTable* inode_table, size_type inode_id);

ErrorCode removeOpenFileEntry(OpenFileTable* open_file_table, char* path_name);

void freeOpenFileTable(OpenFileTable* open_file_table);

void freeInodeTable(InodeTable* inode_table);

void printOpenFileTable(OpenFileTable* open_file_table);

void printInodeTable(InodeTable* inode_table);


