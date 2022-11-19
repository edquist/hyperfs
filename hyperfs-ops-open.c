#include <fuse.h>             // struct fuse_file_info
#include <fcntl.h>            // O_WRONLY, O_RDWR
#include <errno.h>            // EROFS

#include "hyperfs-finfo.h"    // union hyperfs_finfo
#include "hyperfs-cache.h"    // get_cached_path_info_p
#include "logger.h"           // LOG


int hyperfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	LOG("[create: '%s' - nice try.]\n", path);

	return -EROFS;
}

int hyperfs_open(const char *path, struct fuse_file_info *fi)
{
	LOG("[open: '%s']\n", path);

	if (fi->flags & (O_WRONLY|O_RDWR))
		return -EROFS;

	union hyperfs_finfo finfo;
	finfo.ms = get_cached_path_info_p(path);
	fi->fh = finfo.fh;
	return 0;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

