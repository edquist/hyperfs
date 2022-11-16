// #define _XOPEN_SOURCE 500
// #define _GNU_SOURCE

#include <fuse.h>      // struct fuse_operations

#include <stdio.h>     // FILE
#include <errno.h>     // errno
#include <time.h>      // time_t
#include <sys/stat.h>  // struct stat

#include "hyperfs-cache.h"
#include "hyperfs-state.h"
#include "cheddar.h"
#include "logger.h"


int get_http_path_info(
	struct hyperfs_state *remote_info,
	const  char          *path,
	struct ministat      *info)
{

	return 0;
}


// static
int hyperfs_getattr(const char *path, struct stat *stbuf)
{
	int res;

	struct ministat mst;
	if (get_cached_path_info(path, &mst) < 0) {

		// 1. fill mst properly, eg:
		//    get_http_path_info(serverinfo, path, &mst);

		// 2. Then:

		set_cached_path_info(path, &mst);  // ignore failure
	}
	expand_ministat(&mst, stbuf);

	LOG("[getattr: '%s']\n", path);
	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}


struct fuse_operations hyperfs_ops = {
	.getattr = hyperfs_getattr
};


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

