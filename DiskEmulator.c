/*
 *
 */

#include <stdio.h>
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

ErrorCode initDisk(DiskEmulator *disk_emulator){
    int i;
    for(i=0; i<DISK_SIZE; i++){
        disk_emulator->disk[i] = 0;
    }
}

ErrorCode destroyDisk(){
    
}

ErrorCode readBlock(DiskEmulator *disk_emulator, int block_num, void* out_buffer){
    memcpy(out_buffer, &disk_emulator->disk[block_num*BLOCK_SIZE], BLOCK_SIZE);   
}

ErrorCode writeBlock(DiskEmulator *disk_emulator, int block_num, void *in_buffer){
    memcpy(&disk_emulator->disk[block_num*BLOCK_SIZE], in_buffer, BLOCK_SIZE);
}

void printDisk(DiskEmulator *disk_emulator, int block_num){
    int i=block_num*BLOCK_SIZE;
    int j;
    for(j=0; j<BLOCK_SIZE; j++){
        char ch = disk_emulator->disk[j+i];
        printf("%d ", (int)ch);
    }
    printf("\n");
}
