/*
 * This file implements allocInode, garbcollectInode, freeInode, InitInode, getInode.
 * By: Peng
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "InodeAccess.h"
#include "SysCall.h"

size_type getFreeListNum (SuperBlock* super) {
	int i;
	size_type FreeInodesNum=0;
	for (i=0;i<FREE_INODE_NUM;i++){
		if(super->freeInodeList[i]!=-1){
			FreeInodesNum++;
		}
	}
	return FreeInodesNum;
}

ErrorCode allocInode(FileSystem* fs, size_type* inodeId, Inode* inode) {
	//Input is FileSystem* fs
	//WARNING: the caller should define an Inode and use the pointer of that Inode as input to this function
	//If successful, Inode* inode will have inode information according to size_type* inodeId
	//If falied, inodeId and inode will point to nowhere
	
	SuperBlock* super = &(fs->super_block);
	
	if (getFreeListNum(super) > 0){//there are free inodes in the free inodes list cache
		//assign free inode Id
		*inodeId = super->freeInodeList[super->freeInodeIndex];
		
		//update superblock information
		//freeInodeList[super->freeInodeIndex]=-1 means this item is used
		super->freeInodeList[super->freeInodeIndex]=-1;
		super->numOfFreeInodes--;
		super->freeInodeIndex--;
		super->modified = true;
		
		PrintInfo(super, inodeId);
		if (super->freeInodeIndex==-1){
			printf("[Allocate Inode] Free Inodes List Is Empty #>.<#\n");
		}
		
		
		ErrorCode err_getInode=getInode(fs, inodeId, inode);
		
		if (err_getInode==Success){
			printf("[Allocate Inode] Get Inode Successful!\n");
			ErrorCode err_InitInode=InitInode(inode);
			if (err_InitInode==Success){
				printf("[Allocate Inode] Initialize Inode Successful!\n");
				ErrorCode err_putInode=putInode(fs, inodeId, inode);
				if (err_putInode==Success){
					printf("[Allocate Inode] Allocate Inode Successful!\n");
					return Success;						
				}
				else{
					printf("[Allocate Inode] Failure to Put Inode\n");
					return err_putInode;
				}						
			}
			else{
				printf("[Allocate Inode] Failure to Initialize Inode\n");
				return err_InitInode;
			}
		}
		else{
			printf("[Allocate Inode] Falied to Get Inode\n");
			return err_getInode;
		}
	}
	else{//there are no free inodes in the free inodes list cashe, need to garbage collection of the free inodes list cache
		//before garbage collection, we need to shift super->freeInodeIndex to the right index
		
		ErrorCode err_garbcollectInode = garbcollectInode(fs);
		super->freeInodeIndex = FREE_INODE_NUM - 1;
		if (err_garbcollectInode==Success){
			
			//assign free inode Id
			*inodeId = super->freeInodeList[super->freeInodeIndex];						
			
			//update superblock information
			//freeInodeList[super->freeInodeIndex]=-1 means this item is used
			super->freeInodeList[super->freeInodeIndex]=-1;
			super->numOfFreeInodes--;
			super->freeInodeIndex--;
			super->modified = true;
			
			PrintInfo(super, inodeId);
			
			ErrorCode err_getInode=getInode(fs, inodeId, inode);
			if (err_getInode==Success){
				printf("[Allocate Inode] Get Inode Successful!\n");
				ErrorCode err_InitInode=InitInode(inode);
				if (err_InitInode==Success){
					printf("[Allocate Inode] Initialize Inode Successful!\n");
					ErrorCode err_putInode=putInode(fs, inodeId, inode);
					if (err_putInode==Success){
						printf("[Allocate Inode] Allocate Inode Successful!\n");
						return Success;						
					}
					else{
						printf("[Allocate Inode] Failure to Put Inode\n");
						return err_putInode;
					}						
				}
				else{
					printf("[Allocate Inode] Failure to Initialize Inode\n");
					return err_InitInode;
				}
			}
			else{
				printf("[Allocate Inode] Falied to Get Inode\n");
				return err_getInode;
			}
		}
		else{
			printf("[Allocate Inode] After Garbage Collection, Failed to Allocate New Inode\n");
			return err_garbcollectInode;
		}
	}
}




ErrorCode garbcollectInode(FileSystem* fs){
	//Input is FileSystem* fs
	//If successful, the free inodes list cache will be refilled
	
	SuperBlock* super = &(fs->super_block);
	
	size_type num_of_inodes_per_block = BLOCK_SIZE/sizeof(Inode);
	size_type num_of_inode_blocks = super->inodeListSize/BLOCK_SIZE;
	size_type block_no=0;
	size_type insertInodeIndex = FREE_INODE_NUM - 1;//Assume FREE_INODE_NUM is small
	size_type inodeId;
	
	//read block from disk
	void *data_block_buffer;
	data_block_buffer = (void *) malloc (BLOCK_SIZE);
	Inode *temp_inode = data_block_buffer;//pointer to buffer
	bool freed_inode_in_block;//whether there are freed inodes found in blocks
	bool finish = false;
	
	for (;block_no<num_of_inode_blocks;block_no++){
		freed_inode_in_block = false;
		
		//get() is defined in FileSystem.c
		ErrorCode err = get(fs, block_no, data_block_buffer);
		if (err==Success){
			int i=0;
			for (;i<num_of_inodes_per_block;i++){
				inodeId = block_no*num_of_inodes_per_block+i;
				if (getFreeListNum(super) == FREE_INODE_NUM){
					printf("[Garbage Collection] Garbage Collection Search Done :)\n");
					finish = true;
					break;
				}				
				if (temp_inode[i].used==false){
					//freed inode on disk have been found
					freed_inode_in_block = true;
										
					//update super block
					super->freeInodeList[insertInodeIndex]=inodeId;
					super->numOfFreeInodes++;
					insertInodeIndex--;
					super->modified = true;
					
					printf("[Garbage Collection] Inode %ld in Block %ld has been added to free list cache\n",inodeId,block_no);
					printf("[Garbage Collection] Number of Free Inodes: %ld\n",super->numOfFreeInodes);
				}
				else
					continue;
			}
		}
		else{
			printf("[Garbage Collection] Block %ld can not be loaded\n",block_no);
			return Err_GetBlock;
		}
		
		//if inodes in this block have been added to the free list cache, then we should write the block back to the disk
		if (freed_inode_in_block == true){
			//put() is defined in FileSystem.c
			ErrorCode err_put=put(fs, block_no, data_block_buffer);
			if (err_put==Success)
				printf("[Garbage Collection] Block %ld has been written back to disk\n",block_no);
			else{
				printf("[Garbage Collection] Block %ld can not be written back to disk\n",block_no);
				return Err_PutBlock;
			}
		}
		if (finish==true)
				break;
	}
	
	free(data_block_buffer);
	if (super->numOfFreeInodes > 0){
		printf("[Garbage Collection] %ld blocks have been garbage collected :)\n",getFreeListNum(super));
		return Success;
	}
	else{
		printf("[Garbage Collection] All inodes on disk are allocated :(\n");
		return Err_InodeFull;
	}
}





ErrorCode freeInode(FileSystem* fs, size_type* inodeId) {
	//Input is FileSystem* fs, Inode* inode
	//If successful, Inode* inode will be freed on disk
	
	SuperBlock* super = &(fs->super_block);

    //super->numOfFreeInodes records the current number of free inodes, it is a wrong use.
    //Use fs->super_block.numOfInodes - By Yan
    //Corrected by Peng
	if (*inodeId<0 || *inodeId>=fs->super_block.numOfInodes){
		printf("[Free Inode] Inode %ld is not in the legal range :(\n",*inodeId);
		return OutOfBound;
	}
	
	Inode inode;
	ErrorCode err_getInode=getInode(fs, inodeId, &inode);
	if (err_getInode==Success){
		//change the mode of inode so that it could be used for garbage collection
		inode.used=false;
		ErrorCode err_putInode=putInode(fs, inodeId, &inode);
		if (err_putInode==Success){
			printf("[Free Inode] Inode %ld has been freed\n",*inodeId);
			return Success;
		}
		else{
			printf("[Free Inode] Freed Inode %ld can not be written to disk\n",*inodeId);
			return Err_PutInode;
		}
	}
	else{
		printf("[Free Inode] To free Inode %ld, it can not be read from disk\n",*inodeId);
		return Err_GetInode;
	}
}


ErrorCode InitInode(Inode* inode){
	//Input is size_type* inodeId
	//WARNING: the caller should define an Inode and use the pointer of that Inode as input to this function
	//If successful, Inode* inode will be initialized
	inode->used=true;
	inode->fileType=Initialization;
	strcpy (inode->fileOwner, "NULL");
	inode->Permission=777;
	inode->numOfLinks = 0;
	inode->numOfLinks = 0;
	inode->numOfLinks=0;
	inode->fileSize=0;
	
	//time initialization
	time_t current_time;
	char* c_time_string;
	current_time = time(NULL);
	if (current_time == ((time_t)-1)){
		printf("[Initialize Inode] Failure to compute the current time.\n");
		return Err_InitInode;
	}
	c_time_string = ctime(&current_time);
    if (c_time_string == NULL)
    {
        printf("[Initialize Inode] Failure to convert the current time.\n");
        return Err_InitInode;
    }
    strcpy (inode->fileModifiedTime, c_time_string);
    strcpy (inode->fileAccessTime, c_time_string);
    strcpy (inode->inodeModifiedTime, c_time_string);
	
	int i=0;
	for (i=0;i<DIRECT_BLOCK_NUM;i++)
		inode->directBlock[i]=-1;
	inode->singleBlock=-1;
	inode->doubleBlock=-1;
	inode->tripleBlock=-1;
	
	printf("[Initialize Inode] Initialize Inode Successful :)\n");
	return Success;
}

ErrorCode getInode(FileSystem* fs, size_type* inodeId, Inode* inode){
	//Read inode from disk
	//Input is FileSystem* fs, size_type* inodeId
	//WARNING: the caller should define an Inode and use the pointer of that Inode as input to this function
	//If successful, Inode* inode will have inode information according to size_type* inodeId
	
	//To know where the inode is stored in disk, we need to know num_of_inodes_per_block, 
	//block_no(for block reference) and inode_offset(for in-block inode refernece)
	size_type num_of_inodes_per_block = BLOCK_SIZE/sizeof(Inode);
	size_type block_no = *inodeId/num_of_inodes_per_block;
	size_type inode_offset = *inodeId%num_of_inodes_per_block;
	
	//read block from disk
	void *data_block_buffer;
	data_block_buffer = (void *) malloc (BLOCK_SIZE);
	Inode *temp_inode = data_block_buffer;//pointer to buffer
	
	//get() is defined in FileSystem.c
	ErrorCode err = get(fs, block_no, data_block_buffer);
	if (err==Success){
		//WARNING this is a hard copy, buffer will be released later
		*inode = temp_inode[inode_offset];
		free(data_block_buffer);
		return Success;	
	}
	else{
		free(data_block_buffer);
		return Err_GetInode;
	}
}

ErrorCode putInode(FileSystem* fs, size_type* inodeId, Inode* inode){
	//Write inode to disk
	//Input is FileSystem* fs, size_type* inodeId, Inode* inode
	//If successful, Inode* inode will be written to disk according to size_type* inodeId
	
	//To know where the inode is stored in disk, we need to know num_of_inodes_per_block, 
	//block_no(for block reference) and inode_offset(for in-block inode refernece)
	size_type num_of_inodes_per_block = BLOCK_SIZE/sizeof(Inode);
	size_type block_no = *inodeId/num_of_inodes_per_block;
	size_type inode_offset = *inodeId%num_of_inodes_per_block;
	
	//read block from disk
	void *data_block_buffer;
	data_block_buffer = (void *) malloc (BLOCK_SIZE);
	Inode *temp_inode = data_block_buffer;//pointer to buffer
	
	//get() is defined in FileSystem.c
	ErrorCode err = get(fs, block_no, data_block_buffer);
	if (err==Success){
		//WARNING this is a hard copy, buffer will be released later
		temp_inode[inode_offset] = *inode;
		//put() is defined in FileSystem.c
		ErrorCode err_put=put(fs, block_no, data_block_buffer);
		if (err_put==Success){
			free(data_block_buffer);
			return Success;
		}
		else{
			free(data_block_buffer);
			return Err_PutInode;
		}
	}
	else{
		free(data_block_buffer);
		return Err_GetInode;
	}
}



void PrintInfo(SuperBlock* super, size_type* inodeId){
	printf("Inode Allocated: %ld\n",*inodeId);
	printf("Number of Free Inodes: %ld\n",super->numOfFreeInodes);
	printf("Free Inodes Index: %ld\n",super->freeInodeIndex);
	return;
}


ErrorCode readInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* readbyte){
    //read the data refered by inode from disk to buffer (from start to start+size-1), inode is already in memory
    //Input is FileSystem* fs, Inode* inode , size_type start, size_type size
    //If successful, BYTE* buf will be filled
    
    //start and end are data offset in BYTE from direct/indirect blocks in Inode
    size_type end = start + size - 1;
    
    //If the access is out of bound, or size ==0
    if (start<0 || size==0){
    	printf("[Read Inode Data] Access OutofBound)\n");
    	return OutOfBound;
    }
    
    //if read beyond the end of file, then read until end of file
    if (end+1 >= inode->fileSize){
    	end = inode->fileSize-1;
    	//the actual number of BYTEs that have been read
    	*readbyte = end-start+1;
    }
    
    
    size_type block_no; 
    size_type block_offset;
    //everytime read a data block from disk to memory
	void *data_block_buffer;
	data_block_buffer = (void *) malloc (BLOCK_SIZE);
	
	BYTE *data_pt = data_block_buffer;//pointer to buffer
	BYTE *data_out_pt = (BYTE*) buf;//pointer to output buffer
	size_type offset_buf = 0;//output buffer offset
	int flag = 0;//0-first chunk, 1-middle chunk, 2-last chunk
	//int count = 0;//how many blocks have been read
	
    while (start <= end){
    	//ErrorCode Potato_bmap(FileSystem* fs, Inode* inode, size_type* offset, size_type* block_no, size_type* block_offset)
    	//defined in Syscall.c
    	
    	ErrorCode err_readinodedata = Potato_bmap(fs, inode, &start, &block_no, &block_offset);
        printf("cur start block: %ld, offset: %ld\n", block_no, block_offset);
    	if (err_readinodedata == Success){    	    	
    		//get() is defined in FileSystem.c
    		ErrorCode err_get = get(fs, block_no + fs->super_block.firstDataBlockId, data_block_buffer);
    		if (err_get == Success){
    			if ((end-start+1)>=BLOCK_SIZE){
    				if (flag == 0){
    					//the first chunk
    					memcpy(data_out_pt, &data_pt[block_offset], BLOCK_SIZE-block_offset);
    					//update pointer to output buffer
    					offset_buf = BLOCK_SIZE-block_offset;
    					data_out_pt = &buf[offset_buf];
    					//update start position
    					start = start + BLOCK_SIZE-block_offset;
    					//begin reading middle chunks
    					flag = 1;
    				}
    				if (flag == 1){
    					//the middle chunk
    					memcpy(data_out_pt, data_pt, BLOCK_SIZE);
    					//update pointer to output buffer
    					offset_buf = BLOCK_SIZE+offset_buf;
    					data_out_pt = &buf[offset_buf];
    					//update start position
    					start = start + BLOCK_SIZE;
    				}
    			}
    			else{
    				if (flag == 0){
    					//the first chunk
    					memcpy(data_out_pt, &data_pt[block_offset], end-start+1);
    					/*
                        //test if copy corretly for directory only - Yan 
                        DirEntry* entry = (DirEntry*) &data_pt[block_offset];
                        size_type entry_size = 0;
                        while(entry_size<end+start+1){
                            printf("cur dir entry name: %s, inode id: %ld", entry->key, entry->inodeId);
                            entry++;
                            entry_size+=sizeof(DirEntry);
                        }
                        */

                        //update start position (quit while)
    					start = start + BLOCK_SIZE;
    				}
    				if (flag == 1){
    					//the last chunk
    					memcpy(data_out_pt, data_pt, end-start+1);
    					//update start position (quit while)
    					start = start + BLOCK_SIZE;
    				}
    			}
    		}
    		else{
    			printf("[Read Inode Data] Error getting data block\n");
    			free(data_block_buffer);
    			return err_get;
    		}
    	}
    	else{
    		printf("[Read Inode Data] Error calling bmap\n");
    		free(data_block_buffer);
    		return err_readinodedata;
    	}
    }
    free(data_block_buffer);
    printf("[Read Inode Data] Success!\n");
    return Success;
}





ErrorCode writeInodeData(FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size, size_type* writebyte){
    //write the data refered by inode from buffer to disk (from start to start+size-1), inode is already in memory
    //Input is FileSystem* fs, Inode* inode, BYTE* buf, size_type start, size_type size
    //If successful, BYTE* buf will be written to disk
    
    //start and end are data offset in BYTE from direct/indirect blocks in Inode
    size_type end = start + size - 1;
   	//the actual number of BYTEs that have been written
   	*writebyte = end-start+1;    

    //If the access is out of bound, or size ==0
    if (start<0 || size==0){
    	printf("[Write Inode Data] Access OutofBound)\n");
    	return OutOfBound;
    }
    
    //if write beyond the size of file, then inode->fileSize should be modified
    if (end+1 >= inode->fileSize){
    	inode->fileSize = end+1;
    }
    
    size_type block_no; 
    size_type block_offset;
    
    //everytime write a data block from disk to memory, modified the data_block_buffer,
    //then write it back to disk
	void *data_block_buffer;
	data_block_buffer = (void *) malloc (BLOCK_SIZE);	
	BYTE *data_pt = data_block_buffer;//pointer to buffer
		
	BYTE *data_in_pt = (BYTE*) buf;//pointer to input buffer
	size_type offset_buf = 0;//input buffer offset
	
	int flag = 0;//0-first chunk, 1-middle chunk, 2-last chunk
	
    while (start <= end){
    	//ErrorCode Potato_bmap(FileSystem* fs, Inode* inode, size_type* offset, size_type* block_no, size_type* block_offset)
    	//defined in Syscall.c
    	
    	ErrorCode err_writeinodedata = Potato_bmap(fs, inode, &start, &block_no, &block_offset);
    	if (err_writeinodedata == Success){
    		//get() is defined in FileSystem.c
    		ErrorCode err_get = get(fs, block_no, data_block_buffer);
    		if (err_get == Success){
    			if ((end-start+1)>=BLOCK_SIZE){
    				if (flag == 0){
    					//the first chunk
    					memcpy(&data_pt[block_offset], data_in_pt, BLOCK_SIZE-block_offset);
    					//update pointer to output buffer
    					offset_buf = BLOCK_SIZE-block_offset;
    					data_in_pt = &buf[offset_buf];
    					//update start position
    					start = start + BLOCK_SIZE-block_offset;
    					//begin writing middle chunks
    					flag = 1;
    				}
    				if (flag == 1){
    					//the middle chunk
    					memcpy(data_pt, data_in_pt, BLOCK_SIZE);
    					//update pointer to output buffer
    					offset_buf = BLOCK_SIZE+offset_buf;
    					data_in_pt = &buf[offset_buf];
    					//update start position
    					start = start + BLOCK_SIZE;
    				}
    			}
    			else{
    				if (flag == 0){
    					//the first chunk
    					memcpy(&data_pt[block_offset], data_in_pt, end-start+1);
    					//update start position (quit while)
    					start = start + BLOCK_SIZE;
    				}
    				if (flag == 1){
    					//the last chunk
    					memcpy(data_pt, data_in_pt, end-start+1);
    					//update start position (quit while)
    					start = start + BLOCK_SIZE;
    				}
    			}
    			//ErrorCode put(FileSystem *fs, size_type block_no, void* buffer)
    			//defined in FileSystem.c
    			ErrorCode err_put = put(fs, block_no, data_block_buffer);
    			if (err_put != Success){
    				printf("[Write Inode Data] Error putting data block\n");
    				free(data_block_buffer);
    				return err_put;
    			}
    		}
    		else{
    			printf("[Write Inode Data] Error getting data block\n");
    			free(data_block_buffer);
    			return err_get;
    		}
    	}
    	else{
    		printf("[Write Inode Data] Error calling bmap\n");
    		free(data_block_buffer);
    		return err_writeinodedata;
    	}
    }
    free(data_block_buffer);
    printf("[Write Inode Data] Success!\n");
    return Success;
}





