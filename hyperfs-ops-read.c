#include <fuse.h>           // struct fuse_file_info
#include <errno.h>          // EREMOTEIO

#include "hyperfs-finfo.h"  // union hyperfs_finfo
#include "hyperfs-state.h"  // struct hyperfs_state, get_hyperfs_state
#include "getrange.h"       // getrange
#include "loggo.h"          // LOG
#include "ministat.h"       // hyperfs_state's struct ministat ... Hmm.

static inline
size_t min(size_t a, size_t b) { return a <= b ? a : b; }

int hyperfs_read(const char *path, char *buf, size_t size,
                 off_t offset, struct fuse_file_info *fi)
{
	LOG("[read: '%s' size=[%zu] offset=[%ld] ]\n", path, size, offset);

	union hyperfs_finfo finfo = {.fh = fi->fh};

	size_t fsize = finfo.ms->size;
	size_t readsize = fsize <= offset ? 0 : min(size, fsize - offset);
	LOG("[read: fsize=[%zu] readsize=[%zu] ]\n", fsize, readsize);
	if (readsize == 0)
		return 0;

	struct hyperfs_state *remote = get_hyperfs_state();

	FILE *sockf = getrange(remote->host, remote->port, remote->rootpath,
	                       path, offset, &readsize, buf);

	if (sockf) {
		fclose(sockf);
		LOG("[read: retrieved size=[%zu] ]\n", readsize);
		return readsize;
	} else {
		LOG("[read: getrange failed]\n");
		return -EREMOTEIO;
	}
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

