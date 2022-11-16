#include <fuse.h>   // fuse_main

#include <stdio.h>  // printf
#include <stdlib.h> // exit

#include "hyperfs-ops.h"    // hyperfs_ops
#include "hyperfs-cache.h"  // init_cache, free_cache
#include "hyperfs-state.h"  // struct hyperfs_state
#include "logger.h"         // init_logger

void usage(const char *prog)
{
	printf("usage: %s [http://]host[:port][/path/] mountpoint "
	       " [fuse-options]\n", prog);
	exit(0);
}


int main(int argc, char **argv, char **envp)
{
	int res;
	struct hyperfs_state state = {};

	if (argc < 3)
		usage(argv[0]);

	// parse args ...

	init_logger();
	init_cache();
	res = fuse_main(argc, argv, &hyperfs_ops, &state);

	// cleanup as necessary
	free_cache();

	return res;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

