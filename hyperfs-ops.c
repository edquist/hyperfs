#include <fuse.h>                 // struct fuse_operations

#include "hyperfs-ops-getattr.h"  // hyperfs_getattr


struct fuse_operations hyperfs_ops = {
	.getattr = hyperfs_getattr
};


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

