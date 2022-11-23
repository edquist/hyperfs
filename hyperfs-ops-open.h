#pragma once

#include <sys/types.h>  // mode_t

struct fuse_file_info;

int hyperfs_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int hyperfs_open(const char *path, struct fuse_file_info *fi);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

