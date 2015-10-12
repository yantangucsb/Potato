#include "Parameters.h"

typedef struct{
    char disk[DISK_SIZE];
} DiskEmulator; 

ErrorCode initDisk(DiskEmulator *disk_emulator);

ErrorCode destroyDisk();

ErrorCode readBlock(DiskEmulator *disk_emulator, int block_num, void* out_buffer);

ErrorCode writeBlock(DiskEmulator *disk_emulator, int block_num, void *in_buffer);

void printDisk(DiskEmulator *disk_emulator, int block_num);
