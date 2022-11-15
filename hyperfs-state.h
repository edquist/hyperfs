#pragma once

#include <fuse.h>      // fuse_get_context


struct hyperfs_state {

	const char *proto;     // "http"
	const char *host;
	const char *port;      // numeric or service name
	int         port_num;
	int         sockfd;
	FILE       *sockf;
	// cache could go here too, but we leave it to hyperfs-cache
};


static inline struct hyperfs_state *get_hyperfs_state()
{
	return fuse_get_context()->private_data;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

