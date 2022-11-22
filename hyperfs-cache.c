#define _GNU_SOURCE    // hcreate_r, etc
#include <search.h>    // struct hsearch_data, hcreate_r, etc
#include <stdint.h>    // uint64_t
#include <stdio.h>     // perror
#include <stdlib.h>    // exit
#include <string.h>    // strlen, memcpy, memset
#include <sys/stat.h>  // struct stat

#include "ministat.h"  // struct ministat
#include "logger.h"    // LOG

#define MAX_PATH_NODES (1024 * 1024)
#define PATHNAME_BUFSZ (64 * 1024 * 1024)

// TODO: make these smaller buffers/objects (eg 1mb each), but make them
//       linked lists to allow for growth as needed.  the lists never need
//       to be traversed again except for free_cache, as they just provide
//       storage space for the hash table entries (key & data)

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
	if (!ret) {
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

const struct ministat *get_cached_path_info_p(const char *path)
{
	ENTRY e = { (char *) path };  // not modified or stored for lookup
	ENTRY *found = NULL;
	int ret = hsearch_r(e, FIND, &found, &statcache);
	return ret ? found->data : NULL;
}

int get_cached_path_info(const char *path, struct ministat *st)
{
	const struct ministat *msval = get_cached_path_info_p(path);
	if (msval)
		*st = *msval;
	return msval ? 0 : -1;
}

char *get_pathbuf(size_t size)
{
	char *ret = pathbuf_next;
	if (pathbuf_next + size > pathbuf_end)
		return NULL;  // sorry we're full
	pathbuf_next += size;
	return ret;
}

char *add_pathbuf(const char *buf, size_t size)
{
	char *dest = get_pathbuf(size);
	if (dest)
		memcpy(dest, buf, size);
	return dest;
}

// like add_pathbuf, but tack on a NUL terminator, too
char *add_pathlen(const char *buf, size_t len)
{
	size_t size = len + 1;
	char *dest = get_pathbuf(size);
	if (dest) {
		memcpy(dest, buf, len);
		dest[len] = '\0';
	}
	return dest;
}

char *addpath(const char *path)
{
	size_t size = strlen(path) + 1;  // include NUL terminator
	return add_pathbuf(path, size);
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

void expand_ministat(const struct ministat *mst, struct stat *st)
{
	// default 0 for unset fields
	memset(st, 0, sizeof (struct stat));
	switch (mst->type) {
	case S_IFDIR: st->st_mode = 0755 | S_IFDIR; break;
	case S_IFREG: st->st_mode = 0644 | S_IFREG;
	              st->st_size = mst->size;
	              st->st_atime = st->st_mtime = st->st_ctime = mst->mtime;
	              break;
	case S_IFLNK: st->st_mode = 0777 | S_IFLNK;
	              st->st_size = mst->size;
	              break;
	default:
		LOG("[expand_ministat: unknown type: %d]\n", mst->type);
	}
	st->st_nlink = 1;
}

/* vim: set noexpandtab sts=0 sw=8 ts=8: */

