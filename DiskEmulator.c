/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
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


//Phase 2
//Load disk from the existing file
ErrorCode loadDisk(DiskEmulator *disk_emulator){
    disk_emulator->disk = open(DISK_PATH, O_RDWR, 0666);
    if(disk_emulator->disk == -1)
        printf("Disk open failed. Error: %s\n", strerror(errno));
    return Success;
}


ErrorCode initDisk(DiskEmulator *disk_emulator, size_type size){
    disk_emulator->disk = open(DISK_PATH, O_RDWR | O_CREAT, 0666);
    if(disk_emulator->disk == -1)
        printf("Disk init failed. Error: %s\n", strerror(errno));
    if(ftruncate(disk_emulator->disk, size) == -1)
        printf("Set size to disk failed: Error: %s\n", strerror(errno));
    return Success;
}

/*
ErrorCode initDisk(DiskEmulator *disk_emulator, size_type size){
    disk_emulator->disk = malloc(size);
    size_type i;
    for(i=0; i<size; i++){
        disk_emulator->disk[i] = 0;
    }
    return Success;
}
*/

//Phase 2
ErrorCode destroyDisk(DiskEmulator *disk_emulator){
    close(disk_emulator->disk);
}

/*
ErrorCode destroyDisk(DiskEmulator *disk_emulator){
    free(disk_emulator->disk);
    return Success;
}*/


//Phase 2
ErrorCode readBlock(DiskEmulator *disk_emulator, size_type block_num, void* out_buffer){
    size_type offset = block_num*BLOCK_SIZE;
    lseek(disk_emulator->disk, offset, SEEK_SET);
    if(read(disk_emulator->disk, out_buffer, BLOCK_SIZE) == -1)
        printf("Read block %ld failed. Error: %s\n", block_num, strerror(errno));
    return Success;
}

/*
ErrorCode readBlock(DiskEmulator *disk_emulator, size_type block_num, void* out_buffer){
//      sizeof(disk_emulator->disk) is the size of the pointer not the disk
//    if(block_num*BLOCK_SIZE >= sizeof(disk_emulator->disk) || block_num < 0)
//        return OutOfBound;
    memcpy(out_buffer, &(disk_emulator->disk[block_num*BLOCK_SIZE]), BLOCK_SIZE);   
    return Success;
}
*/

//Phase 2
ErrorCode writeBlock(DiskEmulator *disk_emulator, size_type block_num, void *in_buffer){
    size_type offset = block_num*BLOCK_SIZE;
    lseek(disk_emulator->disk, offset, SEEK_SET);
    if(write(disk_emulator->disk, in_buffer, BLOCK_SIZE) == -1)
        printf("Write block %ld failed. Error: %s\n", block_num, strerror(errno));
    return Success;
}

/*
ErrorCode writeBlock(DiskEmulator *disk_emulator, size_type block_num, void *in_buffer){
//    if(block_num*BLOCK_SIZE >= sizeof(disk_emulator->disk) || block_num < 0)
//        return OutOfBound;
    memcpy(&(disk_emulator->disk[block_num*BLOCK_SIZE]), in_buffer, BLOCK_SIZE);
    return Success;
}
*/

//Phase 2
void printDisk(DiskEmulator *disk_emulator, size_type block_num){
    BYTE buf[BLOCK_SIZE];
    if(readBlock(disk_emulator, block_num, buf) == -1)
        printf("Read block %ld failed. Error: %s\n", block_num, strerror(errno));
    size_type j;
    for(j=0; j<BLOCK_SIZE; j++){
        char ch = buf[j];
        printf("%d ", (int)ch);
    }
    printf("\n");
}

/*
void printDisk(DiskEmulator *disk_emulator, size_type block_num){
    size_type i=block_num*BLOCK_SIZE;
    size_type j;
    for(j=0; j<BLOCK_SIZE; j++){
        char ch = disk_emulator->disk[j+i];
        printf("%d ", (int)ch);
    }
    printf("\n");
}*/
