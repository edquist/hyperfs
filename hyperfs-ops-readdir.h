#pragma once

#include <fuse.h>  // fuse_fill_dir_t, struct fuse_file_info


int hyperfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                    off_t offset, struct fuse_file_info *fi);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

