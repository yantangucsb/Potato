/*
 * Test initialization of potato
 * By Yan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "InodeAccess.h"
#include "DataBlockAccess.h"
#include "SysCall.h"

void testNamei(){
    FileSystem fs;
    size_type size = 0x7FFFFFF;
    printf("system size set to: %ld\n", size);
    initFS(size, 20, &fs);
    loadFS(&fs);
   
    initOpenFileTable(&(fs.open_file_table));
    initInodeTable(&(fs.inode_table));

    //create root directory
    Inode inode;
    size_type inode_id = ROOT_INODE_ID;
    if(getInode(&fs, &inode_id, &inode)){
        printf("root directory not exist.\n");
    }

    //allock a block to root directory
    size_type block_id;
    allocBlock(&fs, &block_id);
    printf("[Layer1Test] alloc block %ld to root directory\n", block_id);
    inode.directBlock[0] = block_id;
    inode.fileType = Directory;

    //write 3 entries to root directory
    DirEntry dir_entry[3];
    strcpy(dir_entry[0].key, ".");
    dir_entry[0].inodeId = ROOT_INODE_ID;
    strcpy(dir_entry[1].key, "..");
    dir_entry[1].inodeId = ROOT_INODE_ID;

    strcpy(dir_entry[2].key, "hello");
    size_type second_inodeId;

    //alloc an inode to hello file
    Inode second_inode;
    allocInode(&fs, &second_inodeId, &second_inode);
    printf("inode id for hello is: %ld\n", second_inodeId);

    dir_entry[2].inodeId = second_inodeId;

    //set root directory file size
    inode.fileSize = sizeof(dir_entry);
    printf("size of dir_entry: %ld\n", sizeof(dir_entry));

    //write dir entries to data block
    BYTE* buf = malloc(BLOCK_SIZE);
    memcpy(buf, dir_entry, sizeof(dir_entry));
    put(&fs, block_id + fs.super_block.firstDataBlockId, buf);
    
    //write root directory to disk
    putInode(&fs, &inode_id, &inode);
    
    DirEntry second_dir_entry[4];
    strcpy(second_dir_entry[0].key, ".");
    second_dir_entry[0].inodeId = second_inodeId;
    strcpy(second_dir_entry[1].key, "..");
    second_dir_entry[1].inodeId = ROOT_INODE_ID;
    strcpy(second_dir_entry[2].key, "sec1");

    size_type sec1_inodeId;
    Inode sec1_inode;
    allocInode(&fs, &sec1_inodeId, &sec1_inode);
    printf("inode id for sec1 is: %ld\n", sec1_inodeId);
    second_dir_entry[2].inodeId = sec1_inodeId;

    strcpy(second_dir_entry[3].key, "sec2");

    size_type sec2_inodeId;
    Inode sec2_inode;
    allocInode(&fs, &sec2_inodeId, &sec2_inode);
    printf("inode id for sec2 is: %ld\n", sec2_inodeId);
    second_dir_entry[3].inodeId = sec2_inodeId;
   
    second_inode.fileType = Directory;
    second_inode.fileSize = sizeof(second_dir_entry);
    printf("size of second_dir_entry: %ld\n", sizeof(second_dir_entry));

    size_type second_blockId;

    allocBlock(&fs, &second_blockId);
    memcpy(buf, second_dir_entry, sizeof(second_dir_entry));
    second_inode.directBlock[0] = second_blockId;
    put(&fs, second_blockId + fs.super_block.firstDataBlockId, buf);
    putInode(&fs, &second_inodeId, &second_inode);

    DirEntry sec1_entry[2];
    strcpy(sec1_entry[0].key, ".");
    sec1_entry[0].inodeId = sec1_inodeId;
    strcpy(sec1_entry[1].key, "..");
    sec1_entry[1].inodeId = second_inodeId;

    size_type sec1_blockId;
    allocBlock(&fs, &sec1_blockId);
    memcpy(buf, sec1_entry, sizeof(sec1_entry));
    put(&fs, sec1_blockId + fs.super_block.firstDataBlockId, buf);

    sec1_inode.fileType = Directory;
    sec1_inode.fileSize = sizeof(sec1_entry);
    sec1_inode.directBlock[0] = sec1_blockId;
    putInode(&fs, &sec1_inodeId, &sec1_inode);

    size_type readbyte;
      
    //test if write successfully
    printf("[Layer1Test] test write: block id %ld\n",inode.directBlock[0]); 
    get(&fs, inode.directBlock[0] + fs.super_block.firstDataBlockId, buf);
    DirEntry* cur_entry = (DirEntry*) buf;
    size_type curSize = 0;
    while(curSize < inode.fileSize){
        printf("dir entry name: %s, inode id: %ld\n", cur_entry->key, cur_entry->inodeId);
        cur_entry++;
        curSize += sizeof(DirEntry);
    }
    
    //test readInodeData
    printf("[Read Inode Data] begin readinodedata test..\n");
    readInodeData(&fs, &inode, buf, 0, inode.fileSize, &readbyte);
    printf("[Read Inode Data] readinodedata test successful!\n");

    cur_entry = (DirEntry*) buf;
    curSize = 0;
    while(curSize < inode.fileSize){
        printf("dir entry name: %s, inode id: %ld\n", cur_entry->key, cur_entry->inodeId);
        cur_entry++;
        curSize += sizeof(DirEntry);
    }

    free(buf);

    //test cases for namei
    printf("[Layer1Test] begin namei test..\n");
    if(Potato_namei(&fs, "/", &inode_id) != Success){
        printf("[Layer1Test] Potato_namei failed for path /\n");
    }
    printf("[Layer1Test] root directory inode id: %ld\n", inode_id);
    ErrorCode err = Potato_namei(&fs, "/hello/", &inode_id);
    if(err != Success){
        printf("[Layer1Test] Potato_namei failed for path /hello/: err %d\n", err);
    }
    printf("[Layer1Test] /hello/ direcotry inode id: %ld\n", inode_id);
    err = Potato_namei(&fs, "/..", &inode_id);
    if(err != Success){
        printf("[Layer1Test] Potato_namei failed for path /..: err %d\n", err);
    }
    printf("[Layer1Test] /.. direcotry inode id: %ld\n", inode_id);
    err = Potato_namei(&fs, "/hello/..", &inode_id);
    if(err != Success){
        printf("[Layer1Test] Potato_namei failed for path /hello/..: err %d\n", err);
    }
    printf("[Layer1Test] /hello/.. direcotry inode id: %ld\n", inode_id);
    err = Potato_namei(&fs, "/hello/sec1", &inode_id);
    if(err != Success){
        printf("[Layer1Test] Potato_namei failed for path /hello/sec1: err %d\n", err);
    }
    printf("[Layer1Test] /hello/sec1 direcotry inode id: %ld\n", inode_id);
    err = Potato_namei(&fs, "/hello/sec2", &inode_id);
    if(err != Success){
        printf("[Layer1Test] Potato_namei failed for path /hello/sec2: err %d\n", err);
    }
    printf("[Layer1Test] /hello/sec2 direcotry inode id: %ld\n", inode_id);
    err = Potato_namei(&fs, "/hello/sec1/..", &inode_id);
    if(err != Success){
        printf("[Layer1Test] Potato_namei failed for path /hello/sec1/..: err %d\n", err);
    }
    printf("[Layer1Test] /hello/sec1/.. direcotry inode id: %ld\n", inode_id);
    err = Potato_namei(&fs, "/hello/sec1/../..", &inode_id);
    if(err != Success){
        printf("[Layer1Test] Potato_namei failed for path /hello/sec1/../..: err %d\n", err);
    }
    printf("[Layer1Test] /hello/sec1/../.. direcotry inode id: %ld\n", inode_id);
    err = Potato_namei(&fs, "/hello/sec1/./..", &inode_id);
    if(err != Success){
        printf("[Layer1Test] Potato_namei failed for path /hello/sec1/./..: err %d\n", err);
    }
    printf("[Layer1Test] /hello/sec1/./.. direcotry inode id: %ld\n", inode_id);
}

void testBmap(){
    FileSystem fs;
    size_type size = 0x7FFFFFF;
    printf("system size set to: %ld\n", size);
    initFS(size, 20, &fs);
//    printDisk(&(fs.disk_emulator), 0);
    printf("size of Inode: %lu\n", sizeof(Inode));
    printf("size of SuperBlock: %lu\n", sizeof(SuperBlock));
//    readSuperBlock(&fs);
//    printFileSystem(&fs);
    
    //test Potato_bmap
    Inode inode;
    size_type i = 0;
    size_type curIndex = 0;
    
    for(i=0; i<DIRECT_BLOCK_NUM; i++){
        inode.directBlock[i] = i+curIndex;
    }
    
    printf("Assign block id to directBlock.\n");
    
    size_type block_id;
    allocBlock(&fs, &block_id);
    inode.singleBlock = block_id;

    curIndex += DIRECT_BLOCK_NUM;
    size_type arr[BLOCK_SIZE/sizeof(size_type)];
    for(i=0; i<BLOCK_SIZE/sizeof(size_type); i++){
        arr[i] = curIndex + i;
    }
    put(&fs, block_id + fs.super_block.firstDataBlockId, arr);

    printf("Assign block id to singleBlock.\n");
    
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
    
    printf("Assign block id to doubleBlock.\n");
    
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
                tmptmpArr[k] = k+curIndex;
                if(tmptmpArr[k] == 131072){
                    printf("cur i= %ld, cur j=%ld, cur k= %ld\n", i, j, k);
                }
            }
            curIndex += BLOCK_SIZE/sizeof(size_type);
            put(&fs, tmptmpBlock_id + fs.super_block.firstDataBlockId, tmptmpArr);
        }
        put(&fs, tmpBlock_id + fs.super_block.firstDataBlockId, tmpArr);
    }

    put(&fs, block_id + fs.super_block.firstDataBlockId, arr);

    printf("Assign block id to tripleBlock.\n");
    printf("Total num of block id: %ld\n", curIndex);  
    //test cases
    size_type offset[] = {0, 1, 512, 512*2, 512*10, 512*20, 512*512, 512*512+2, 512*512*512};
    
    for(i=0; i<sizeof(offset)/sizeof(size_type); i++){
        size_type offsetInBlock = 0;
    //    printf("Getting block id for offest %ld\n", offset[i]);
        if(Potato_bmap(&fs, &inode, &offset[i], &block_id, &offsetInBlock) != Success){
            printf("test case %ld: Potato_bmap failed\n", *(offset+i));
        }
        printf("test case %ld: block id -- %ld, offset -- %ld\n", *(offset+i), block_id, offsetInBlock);
    }

}

int main(int argc, char *argv[]){
    //testBmap();
    testNamei();
    return 0;
}


