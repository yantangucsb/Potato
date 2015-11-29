/*
 * This is Inode
 * By Yan
 */

#pragma once
#include <stdbool.h>
#include "sys/types.h"
#include "time.h"
#include "string.h"
#include "Parameters.h"

typedef struct Inode{
    //Reserve the space for the flag of used
    bool used;

    //File owner identifer: individual owner
    char fileOwner[FILE_OWNER_LENGTH];

    //File type: regular '-', directory 'd', character device file 'c', block device file 'b'
    //local socket file 's', FIFO (pipes) 'p', symbolic link 'l'.
    FileType fileType;
	
	uid_t _in_uid;
	gid_t _in_gid;
	
    //File access permissions: Three classes -- the owner, the group owner and other users.
    //The access rights for each of the three classes can be set individually: read, write and execute
    //Execution permission for a directory gives the right to search the directory for a file name
    //write permission affects syscall: create, mknod, link, unlink
    //read:4 write:2 execute:1; Owner-Group-Others (x,x,x)
    uint32_t Permission;

    //File access times: the time the file was last modified, when it was last accessed
    //and when the inode was last modified
    LONG fileModifiedTime;
    LONG fileAccessTime;
    LONG inodeModifiedTime;

    //Number of links to the file: The number of names the file has in the directory hierarchy
    size_type numOfLinks;

    //Table of contents for the disk addresses of data in a file
    size_type directBlock[DIRECT_BLOCK_NUM];
    size_type singleBlock;
    size_type doubleBlock;
    size_type tripleBlock;

    //File size -- It is 1 greater than the highest byte offset of data in the file
    size_type fileSize;
} Inode;
