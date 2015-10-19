/*
 * This is the inmemory buffer interface for Layer 0
 * By Yan
 */

#include "Parameters.h"

typedef struct{
    BYTE* disk;
} DiskEmulator; 

ErrorCode initDisk(DiskEmulator *disk_emulator, size_type size);

ErrorCode destroyDisk(DiskEmulator *);

ErrorCode readBlock(DiskEmulator *disk_emulator, int block_num, void* out_buffer);

ErrorCode writeBlock(DiskEmulator *disk_emulator, int block_num, void *in_buffer);

void printDisk(DiskEmulator *disk_emulator, int block_num);
