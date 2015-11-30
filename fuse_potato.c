/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall fuse_potato.c `pkg-config fuse --cflags --libs` -o fuse_potato
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
//#include "Directory.h"
#include "SysCall.h"
#include "FileSystem.h"

static FileSystem fs;
static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static int f_getattr(const char *path, struct stat *stbuf)
{
	printf("[Potato_getattr] calling getattr\n");
	int res = 0;
	#ifdef DEBUG
	//printf("getattr %s\n", path);
	#endif
	memset(stbuf, 0, sizeof(struct stat));
	return Potato_getattr(&fs, path, stbuf);
}

static int f_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	printf("[Potato_readdir] calling readdir\n");
	//printf("offset: %u\n", offset);
	(void) fi;
 	DirEntry curEntry;
	//UINT entryL = (24 + FILE_NAME_LENGTH + 7) & (~7);
	
	INT res = Potato_readdir(&fs, path, offset, &curEntry);
	if (res == -1)
		return -ENOENT;
	while (res == 0) {
		if (curEntry.inodeId != -1) {
			//printf("filling %s\n", curEntry.key);
			offset ++;
			if (filler(buf, curEntry.key, NULL, offset) == 1) {
				//printf("fuse_filler buf full!\n");
				break;
			}
			//printf("%s\n", (char *)buf);
			return 0;
		}
		offset ++;
		res = Potato_readdir(&fs, path, offset, &curEntry);
	}
	return 0;
}

static int f_mknod(const char *path, mode_t mode, dev_t dev)
{
	printf("[Potato_mknod] calling mknod\n");
	struct fuse_context* fctx = fuse_get_context();
	INT res = Potato_mknod(&fs, path, fctx->uid, fctx->gid);
	return res>0?0:res;
}

static int f_mkdir(const char *path, mode_t mode)
{
	printf("[Potato_mkdir] calling mkdir\n");
	struct fuse_context* fctx = fuse_get_context();
	INT res = Potato_mkdir(&fs, path, fctx->uid, fctx->gid);
	return res>0?0:res;
}

static int f_unlink(const char *path)
{
	printf("[Potato_unlink] calling unlink\n");
	return Potato_unlink(&fs, path);
}

static int f_rmdir(const char *path)
{
	printf("[Potato_rmdir] calling rmdir\n");
	return Potato_unlink(&fs, path);
}

static int f_rename(const char *path, const char *new_path)
{
	printf("[Potato_rename] calling rename\n");
	#ifdef DEBUG
	printf("old name: %s\n", path);
	printf("new name: %s\n", new_path);
	#endif
	return Potato_rename(&fs, path, new_path);
}

static int f_chmod(const char *path, mode_t mode)
{
	printf("[Potato_chmod] calling chmod\n");
	#ifdef DEBUG
	printf("l3_chmod with mode: %x\n", mode);
	#endif
	return Potato_chmod(&fs, path, mode);
}

static int f_chown(const char *path, uid_t uid, gid_t gid)
{
	printf("[Potato_chown] calling chown\n");
	printf("l3_chown\n");
	return Potato_chown(&fs, path, uid, gid);
}

static int f_truncate(const char *path, off_t offset)
{
	printf("[Potato_truncate] calling truncate\n");
    //printf("truncate %s to be length %u\n", path, offset);
	return Potato_truncate(&fs, path, offset);;
}

static int f_open(const char *path, struct fuse_file_info *fi)
{
	printf("[Potato_open] calling open\n");
    //parse file operation from flags
    FileOp fileOp;
    int opflag = fi->flags & 3;
    if(opflag == O_RDONLY)
        fileOp = READ;
    else if(opflag == O_WRONLY)
        fileOp = WRITE;
    else if(opflag == O_RDWR)
        fileOp = READWRITE;
    else {
        fprintf(stderr, "Error: no file operation specified in flags!\n");
        return -EINVAL;
    }
    printf("File operation from flags: %d\n", fileOp);
    
    //parse create flag
    if(fi->flags & O_CREAT) {
        #ifdef DEBUG
        printf("O_CREAT flag detected, creating file: %s\n", path);
        #endif
        struct fuse_context* fctx = fuse_get_context();
        INT succ = Potato_mknod(&fs, path, fctx->uid, fctx->gid);
        
        //parse exists flag
        if((fi->flags & O_EXCL) && (succ == -EEXIST)) {
            fprintf(stderr, "Error: O_EXCL specified for open but file already exists!\n");
            return -EEXIST;
        }
        else if(succ < 0) {
            return succ;
        }
    }
    
    //parse truncate flag
    if(fi->flags & (O_TRUNC | O_WRONLY | O_RDWR)) {
        #ifdef DEBUG
        printf("O_TRUNC flag detected, truncating file: %s\n", path);
        #endif
        //TODO truncate file
    }
    
	return (int)Potato_open(&fs, path, fileOp);
}

static int f_release(const char *path, struct fuse_file_info *fi)
{
	printf("[Potato_release] calling releases\n");
    //parse file operation from flags
    FileOp fileOp;
    int opflag = fi->flags & 3;
    if(opflag == O_RDONLY)
        fileOp = READ;
    else if(opflag == O_WRONLY)
        fileOp = WRITE;
    else if(opflag == O_RDWR)
        fileOp = READWRITE;
    else {
        fprintf(stderr, "Error: no file operation specified in flags!\n");
        return -EINVAL;
    }
    #ifdef DEBUG
    printf("File operation from flags: %d\n", fileOp);
    #endif
    
	return (int)Potato_close(&fs, path, fileOp);
}

static int f_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	printf("[Potato_read] calling read\n");
        /*
	size_t len;
	(void) fi;
	if(strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;
	*/
	#ifdef DEBUG
	printf("Calling Potato_read for path \"%s\" and offset: %u for size: %u\n", path, offset, size);
	#endif
	return (int)Potato_read(&fs, path, offset, buf, size);
}

static int f_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("[Potato_write] calling write\n");
	#ifdef DEBUG_VERBOSE
    	printf("l3_write received buffer to write: %s\n", buf);
	printf("Calling Potato_write for path \"%s\" and offset: %u for size: %u\n", path, offset, size);
	#endif
	return (int)Potato_write(&fs, path, offset, buf, size);
}

static int f_utimens(const char *path, const struct timespec tv[2]) 
{
	return Potato_utimens(&fs, path, tv);
}

static int f_statfs(const char *path, struct statvfs *stat)
{
	printf("[Potato_statfs] calling statfs\n");
	;
}

void * f_mount(struct fuse_conn_info *conn)
{
	printf("[Potato_mount] calling mount\n");
	INT succ = Potato_mount(&fs);
}

void * f_unmount(void *conn)
{
	printf("[Potato_unmount] calling unmount\n");
	INT succ = Potato_unmount(&fs);
}

static struct fuse_operations Potato_oper = {
	.getattr	= f_getattr,
	.readdir	= f_readdir,
	.mknod		= f_mknod,
	.mkdir		= f_mkdir,
	.unlink		= f_unlink,
	.rmdir		= f_rmdir,
	.rename		= f_rename,
	.chmod		= f_chmod,
	.chown		= f_chown,
	.truncate	= f_truncate,
	.open		= f_open,
	.release	= f_release,
	.read		= f_read,
	.write		= f_write,
//	.utimens	= f_utimens,
	.statfs		= f_statfs,
//	.init		= l3_mount,
	.destroy	= f_unmount
};

int main(int argc, char *argv[])
{
	/*printf("Initializing file system with initfs...\n");
    	UINT succ = Potato_initfs(128, 16, &fs);
    	if(succ == 0) {
        	printf("initfs succeeded with filesystem size: %d\n", fs.nBytes);
    	}
    	else {
        	printf("Error: initfs failed with error code: %d\n", succ);
    	}*/
	INT succ = Potato_mount(&fs);
	return fuse_main(argc, argv, &Potato_oper, NULL);	
}

