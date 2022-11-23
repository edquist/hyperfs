#include <string.h>         // memcpy

#include "hyperfs-cache.h"  // get_cached_path_info_p
#include "ministat.h"       // struct ministat
#include "loggo.h"          // LOG

int hyperfs_readlink(const char *path, char *buf, size_t size)
{
	LOG("[readlink: '%s' size=[%zu] ]\n", path, size);

	// trust library did getattr already, so we assume success
	const struct ministat *ms = get_cached_path_info_p(path);

	size_t len = ms->size < size ? ms->size : size - 1;
	memcpy(buf, ms->link, len);
	buf[len] = '\0';

	return 0;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

