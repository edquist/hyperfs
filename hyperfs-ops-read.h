#pragma once

#include <stddef.h>     // size_t
#include <sys/types.h>  // off_t


struct fuse_file_info;

int hyperfs_read(const char *path, char *buf, size_t size,
                 off_t offset, struct fuse_file_info *fi);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */


