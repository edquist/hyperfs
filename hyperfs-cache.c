#define _GNU_SOURCE    // hcreate_r, etc
#include <search.h>    // hcreate_r, etc
#include <stdint.h>    // uint64_t
#include <stdio.h>     // perror
#include <stdlib.h>    // exit
#include <string.h>    // strlen, memcpy

#include "hyperfs-cache.h"  // struct ministat


#define MAX_PATH_NODES (1024 * 1024)
#define PATHNAME_BUFSZ (64 * 1024 * 1024)

static struct hsearch_data statcache;
static char  *pathbuf;
static char  *pathbuf_next;
static char  *pathbuf_end;

static struct ministat *statbuf;
static struct ministat *statbuf_next;
static struct ministat *statbuf_end;


void init_cache()
{
	int ret = hcreate_r(MAX_PATH_NODES, &statcache);
	if (ret) {
		perror("hcreate_r");
		exit(1);
	}
	pathbuf = malloc(PATHNAME_BUFSZ);
	pathbuf_next = pathbuf;
	pathbuf_end  = pathbuf + PATHNAME_BUFSZ;
	if (!pathbuf) {
		perror("malloc");
		exit(1);
	}
	statbuf = malloc(MAX_PATH_NODES * sizeof(struct ministat));
	statbuf_next = statbuf;
	statbuf_end  = statbuf + MAX_PATH_NODES;
	if (!statbuf) {
		perror("malloc");
		exit(1);
	}
}

void free_cache()
{
	hdestroy_r(&statcache);
	free(pathbuf);
	free(statbuf);
}

int get_cached_path_info(const char *path, struct ministat *st)
{
	ENTRY e = { (char *) path };  // not modified or stored for lookup
	ENTRY *found = NULL;
	int ret = hsearch_r(e, FIND, &found, &statcache);
	if (ret) {
		struct ministat *msval = found->data;
		*st = *msval;
	}
	return ret ? 0 : -1;
}

static
char *addpath(const char *path)
{
	size_t len = strlen(path) + 1;  // include NUL terminator
	char *ret = pathbuf_next;
	if (pathbuf_next + len > pathbuf_end)
		return NULL;  // sorry we're full
	memcpy(pathbuf_next, path, len);
	pathbuf_next += len;
	return ret;
}

static
struct ministat *addstat(const struct ministat *st)
{
	struct ministat *ret = statbuf_next;
	if (statbuf_next >= statbuf_end)
		return NULL;  // sorry we're full
	*statbuf_next++ = *st;
	return ret;
}

int set_cached_path_info(const char *path, const struct ministat *st)
{
	ENTRY e;
	e.key = addpath(path);
	e.data = addstat(st);
	if (!e.key || !e.data)
		return -1;  // sorry we're full
	ENTRY *found = NULL;
	int ret = hsearch_r(e, ENTER, &found, &statcache);
	if (ret) {
		// found points to new entry ... not that we use it though.
	}
	return ret ? 0 : -1;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

