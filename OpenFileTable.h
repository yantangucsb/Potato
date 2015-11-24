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
    InodeEntry* tail;
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
    InodeEntry* inodeEntry;
    OpenFileEntry* next;
};

typedef struct OpenFileTable{
    size_type nOpenFiles;
    OpenFileEntry* head;
} OpenFileTable;

ErrorCode getOpenFileEntry(OpenFileTable* open_file_table, char* path_name, OpenFileEntry* entry);
