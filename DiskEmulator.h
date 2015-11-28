/*
 * This is the inmemory buffer interface for Layer 0
 * NOTICE: Please do not call any interface from this file in any file other than FileSystem.*
 *          for sakes of changing interfaces in Layer2
 * By Yan
 */

#include "Parameters.h"

//Phase 2
typedef struct{
    INT disk;
} DiskEmulator;

/*
//Phase 1
typedef struct{
    BYTE* disk;
} DiskEmulator; 
*/
ErrorCode initDisk(DiskEmulator *disk_emulator, size_type size);

ErrorCode destroyDisk(DiskEmulator *);

ErrorCode readBlock(DiskEmulator *disk_emulator, size_type block_num, void* out_buffer);

ErrorCode writeBlock(DiskEmulator *disk_emulator, size_type block_num, void *in_buffer);

void printDisk(DiskEmulator *disk_emulator, size_type block_num);
