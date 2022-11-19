#pragma once

#include <fuse.h>      // fuse_get_context


struct hyperfs_state {

	char *proto;     // "http"
	char *host;
	char *port;      // numeric or service name
	char *rootpath;  // does not include trailing slash
	int   port_num;
	int   sockfd;
	FILE *sockf;
	// cache could go here too, but we leave it to hyperfs-cache
};


static inline struct hyperfs_state *get_hyperfs_state()
{
	return fuse_get_context()->private_data;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

