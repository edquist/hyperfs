#pragma once

#include <stdio.h>     // FILE  ... sorry.
#include <fuse.h>      // fuse_get_context

struct addrinfo;

struct hyperfs_state {

	char *proto;     // "http"
	char *host;
	char *port;      // numeric or service name
	char *rootpath;  // does not include trailing slash
	FILE *sockf;
	struct addrinfo *ainfo;
	int   tz_offset; // against dir timestamps from index listings
	int   tz_offset_found;

	// cache could go here too, but we leave it to hyperfs-cache
};


static inline struct hyperfs_state *get_hyperfs_state()
{
	return fuse_get_context()->private_data;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

