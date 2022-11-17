#define _GNU_SOURCE    // hcreate_r, etc
#include <search.h>    // struct hsearch_data, hcreate_r, etc
#include <stdint.h>    // uint64_t
#include <stdio.h>     // perror
#include <stdlib.h>    // exit
#include <string.h>    // strlen, memcpy, memset
#include <sys/stat.h>  // struct stat

#include "ministat.h"  // struct ministat


#define MAX_PATH_NODES (1024 * 1024)
#define PATHNAME_BUFSZ (64 * 1024 * 1024)
#define LISTNODE_BUFSZ (1024 * 1024)
#define LIST_BUFSZ     (LISTNODE_BUFSZ - sizeof (void *))

// TODO: make these smaller buffers/objects (eg 1mb each), but make them
//       linked lists to allow for growth as needed.  the lists never need
//       to be traversed again except for free_cache, as they just provide
//       storage space for the hash table entries (key & data)

static struct hsearch_data statcache;

struct pathbuf_list {
	struct pathbuf_list *next_list;
	char                 buf[LIST_BUFSZ];
	char                 bufend[0];
};

struct statbuf_list {
	struct statbuf_list *next_list;
	struct ministat      buf[LIST_BUFSZ / sizeof (struct ministat)];
	struct ministat      bufend[0];
};

static struct pathbuf_list *pathbuf0;
static struct pathbuf_list *pathbuf_cur;
static char                *pathbuf_buf;

static struct statbuf_list *statbuf0;
static struct statbuf_list *statbuf_cur;
static struct ministat     *statbuf_buf;

#if 0
static char  *pathbuf;
static char  *pathbuf_next;
static char  *pathbuf_end;

static struct ministat *statbuf;
static struct ministat *statbuf_next;
static struct ministat *statbuf_end;
#endif


void init_cache()
{
	int ret = hcreate_r(MAX_PATH_NODES, &statcache);
	if (ret) {
		perror("hcreate_r");
		exit(1);
	}
	pathbuf0 = pathbuf_cur = malloc(sizeof (struct pathbuf_list));
	statbuf0 = statbuf_cur = malloc(sizeof (struct statbuf_list));
	if (!pathbuf0 || !statbuf0) {
		perror("malloc");
		exit(1);
	}
	pathbuf_cur->next_list = NULL;
	pathbuf_buf = pathbuf_cur->buf;

	statbuf_cur->next_list = NULL;
	statbuf_buf = statbuf_cur->buf;

#if 0
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
#endif
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

void expand_ministat(const struct ministat *mst, struct stat *st)
{
	memset(st, 0, sizeof (struct stat));
	st->st_mode = mst->type == S_IFDIR ? 0755 | S_IFDIR
	                                   : 0644 | S_IFREG;
	st->st_nlink = 1;
	st->st_size  = mst->size;
	st->st_atime = mst->mtime;
	st->st_mtime = mst->mtime;
	st->st_ctime = mst->mtime;
}

/* vim: set noexpandtab sts=0 sw=8 ts=8: */

