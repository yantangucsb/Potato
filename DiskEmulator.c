/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/mman.h>
#include "DiskEmulator.h"

/*ErrorCode initDisk(){
    FILE *fp;
    fp = fopen("diskFile", "w");
    if(fp == NULL){
        printf("File not exists.\n");
    }
    truncate(fileno(fp), DISK_SIZE); 
    
    DiskEmulator de;
    ErrorCode *p = mmap(&de, DISK_SIZE, PROT_WRITE, MAP_PRIVATE, fileno(fp), 0);
    if(p == MAP_FAILED)
            perror ("What? mmap failed");
    destroyDisk();
    fclose(fp);
}*/

ErrorCode initDisk(DiskEmulator *disk_emulator, size_type size){
    disk_emulator->disk = malloc(size);
    size_type i;
    for(i=0; i<size; i++){
        disk_emulator->disk[i] = 0;
    }
    return Success;
}

ErrorCode destroyDisk(DiskEmulator *disk_emulator){
    free(disk_emulator->disk);
    return Success;
}

ErrorCode readBlock(DiskEmulator *disk_emulator, size_type block_num, void* out_buffer){
//      sizeof(disk_emulator->disk) is the size of the pointer not the disk
//    if(block_num*BLOCK_SIZE >= sizeof(disk_emulator->disk) || block_num < 0)
//        return OutOfBound;
    memcpy(out_buffer, &disk_emulator->disk[block_num*BLOCK_SIZE], BLOCK_SIZE);   
    return Success;
}

ErrorCode writeBlock(DiskEmulator *disk_emulator, size_type block_num, void *in_buffer){
//    if(block_num*BLOCK_SIZE >= sizeof(disk_emulator->disk) || block_num < 0)
//        return OutOfBound;
    memcpy(&disk_emulator->disk[block_num*BLOCK_SIZE], in_buffer, BLOCK_SIZE);
    return Success;
}

void printDisk(DiskEmulator *disk_emulator, size_type block_num){
    size_type i=block_num*BLOCK_SIZE;
    size_type j;
    for(j=0; j<BLOCK_SIZE; j++){
        char ch = disk_emulator->disk[j+i];
        printf("%d ", (int)ch);
    }
    printf("\n");
}
