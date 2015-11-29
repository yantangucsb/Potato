/*
 *Test Layer 0 by write a block and read a block
 *By Yan
 */

#include <stdlib.h>
#include <stdio.h>
#include "DiskEmulator.h"

int main(int argc, char *argv[]){
   DiskEmulator diskEmulator;
   initDisk(&diskEmulator, BLOCK_SIZE*16);

   char data[BLOCK_SIZE];
   int i;
   for(i=0; i<BLOCK_SIZE; i++){
        data[i] = (char)i;
   }
   writeBlock(&diskEmulator, 4, data);
   printf("Block 4 data on disk:\n");
   printDisk(&diskEmulator, 4);
   BYTE out_buffer[BLOCK_SIZE];
   readBlock(&diskEmulator, 4, out_buffer);
   printf("Test readBlock on block 4:\n");
   for(i=0; i<BLOCK_SIZE; i++){
       char ch = out_buffer[i];
       printf("%d ", (int)ch);
   }
   printf("\n");
   destroyDisk(&diskEmulator);
   return 0;
}
