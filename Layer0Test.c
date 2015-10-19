/*
 *Test Layer 0 by write a block and read a block
 *By Yan
 */

#include <stdlib.h>
#include <stdio.h>
#include "DiskEmulator.h"

int main(int argc, char *argv[]){
   DiskEmulator diskEmulator;
   initDisk(&diskEmulator, 256*16);

   char data[BLOCK_SIZE];
   int i;
   for(i=0; i<BLOCK_SIZE; i++){
        data[i] = (char)i;
   }
   writeBlock(&diskEmulator, 4, &data);
   printDisk(&diskEmulator, 4);
   char* out_buffer = malloc(BLOCK_SIZE);
   readBlock(&diskEmulator, 4, out_buffer);
   for(i=0; i<BLOCK_SIZE; i++){
       char ch = *(out_buffer+i);
       printf("%d ", (int)ch);
   }
   printf("\n");
   destroyDisk(&diskEmulator);
   return 0;
}
