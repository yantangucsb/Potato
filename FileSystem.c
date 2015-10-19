/*
 * Set up the file system
 * By Yan
 */

#include "FileSystem.h"

/*
 *size -- System size
 *percen -- percentage of inode space on disk, e.g. 10 means 10%
 *fs -- in memory buffer for files ystem 
 */
ErrorCode initFS(size_type size, size_type percen, FileSystem* fs){
    //set up super block
    initSuperBlock(size, percen, &(fs->super_block));
   
    initDisk(fs->disk_emulator);
    //put everything on disk
    writeBlock(fs->disk_emulator, SUPER_BLOCK_OFFSET, &(fs->super_block));
}
