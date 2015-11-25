/*
 * Test initialization of potato
 * By Yan
 */

#include <stdio.h>
#include "InodeAccess.h"
#include "DataBlockAccess.h"
#include "SysCall.h"

int main(int argc, char *argv[]){
    FileSystem fs;
    size_type size = 0x7FFFFFF;
    printf("system size set to: %ld\n", size);
    initFS(size, 20, &fs);
//    printDisk(&(fs.disk_emulator), 0);
    printf("size of Inode: %lu\n", sizeof(Inode));
    printf("size of SuperBlock: %lu\n", sizeof(SuperBlock));
//    readSuperBlock(&fs);
//    printFileSystem(&fs);
    
    //test bmap
    Inode inode;
    size_type i = 0;
    size_type curIndex = 0;
    for(i=0; i<DIRECT_BLOCK_NUM; i++){
        inode.directBlock[i] = i+curIndex;
    }
    size_type block_id;
    allocBlock(&fs, &block_id);
    inode.singleBlock = block_id;

    curIndex += DIRECT_BLOCK_NUM;
    size_type arr[BLOCK_SIZE/sizeof(size_type)];
    for(i=0; i<BLOCK_SIZE/sizeof(size_type); i++){
        arr[i] = curIndex + i;
    }
    put(&fs, block_id + fs.super_block.firstDataBlockId, arr);

    curIndex += BLOCK_SIZE/sizeof(size_type);
    allocBlock(&fs, &block_id);
    inode.doubleBlock = block_id;

    for(i=0; i<BLOCK_SIZE/sizeof(size_type); i++){
        size_type tmpBlock_id;
        allocBlock(&fs, &tmpBlock_id);
        arr[i] = tmpBlock_id;
        size_type tmpArr[BLOCK_SIZE/sizeof(size_type)];
        size_type j;
        for(j=0; j<BLOCK_SIZE/sizeof(size_type); j++){
            tmpArr[j] = j+curIndex;
        }
        curIndex += BLOCK_SIZE/sizeof(size_type);
        put(&fs, tmpBlock_id + fs.super_block.firstDataBlockId, tmpArr);
    }

    put(&fs, block_id + fs.super_block.firstDataBlockId, arr);
    allocBlock(&fs, &block_id);
    inode.tripleBlock = block_id;
    for(i=0; i<BLOCK_SIZE/sizeof(size_type); i++){
        size_type tmpBlock_id;
        allocBlock(&fs, &tmpBlock_id);
        arr[i] = tmpBlock_id;
        size_type tmpArr[BLOCK_SIZE/sizeof(size_type)];
        size_type j;
        for(j=0; j<BLOCK_SIZE/sizeof(size_type); j++){
            size_type tmptmpBlock_id;
            allocBlock(&fs, &tmptmpBlock_id);
            tmpArr[j] = tmptmpBlock_id;
            size_type tmptmpArr[BLOCK_SIZE/sizeof(size_type)];
            size_type k;
            for(k=0; k<BLOCK_SIZE/sizeof(size_type); k++){
                tmptmpArr[k] = i+curIndex;
            }
            curIndex += BLOCK_SIZE/sizeof(size_type);
            put(&fs, tmptmpBlock_id + fs.super_block.firstDataBlockId, tmptmpArr);
        }
        put(&fs, tmpBlock_id + fs.super_block.firstDataBlockId, tmpArr);
    }

    put(&fs, block_id + fs.super_block.firstDataBlockId, arr);

    //test cases
    size_type offset[] = {0, 1, 512, 512*2, 512*10, 512*20, 512*512, 512*512*512};
    
    for(i=0; i<sizeof(offset); i++){
        size_type offsetInBlock = 0;
        if(bmap(&fs, &inode, &offset[i], &block_id, &offsetInBlock) != Success){
            printf("test case %ld: bmap failed", *(offset+i));
        }
        printf("test case %ld: block id -- %ld, offset -- %ld", *(offset+i), block_id, offsetInBlock);
    }

    return 0;
}
