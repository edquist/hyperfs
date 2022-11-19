#include <fuse.h>                 // struct fuse_operations

#include "hyperfs-ops-getattr.h"  // hyperfs_getattr
#include "hyperfs-ops-open.h"     // hyperfs_read


struct fuse_operations hyperfs_ops = {
	.getattr = hyperfs_getattr,
	.create  = hyperfs_create,
	.open    = hyperfs_open
};


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

