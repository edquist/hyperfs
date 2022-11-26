#include <stdio.h>     // FILE
#include <stdlib.h>    // malloc, free :(
#include <errno.h>     // errno, EREMOTEIO
#include <time.h>      // time_t
#include <sys/stat.h>  // S_IFREG, S_IFDIR, S_IFLNK
#include <string.h>    // strerror, strstr

#include "hyperfs-cache.h"  // get_cached_path_info, set_cached_path_info
#include "hyperfs-state.h"  // struct hyperfs_state, get_hyperfs_state
#include "ministat.h"       // get_cached_path_info, set_cached_path_info
#include "cheddar.h"        // struct resp_info, get_resp_info
#include "connor.h"         // tcp_connect
#include "loggo.h"          // LOG
#include "sendo.h"          // SENDO, S_ENDO
#include "escape.h"         // escape_raw, path_needs_escape

static
void send_head_plain(
	FILE                       *sockf,
	const struct hyperfs_state *remote,
	const char                 *path)
{
	SENDO(sockf, "HEAD %s%s HTTP/1.1", remote->rootpath, path);
	SENDO(sockf, "Host: %s", remote->host);  // skip port
	SENDO(sockf, "Connection: close");
	SENDO(sockf, "");

	if (fflush(sockf)) {
		LOG("[send_get_plain: fflush failed; is connection closed?]\n");
		perror("fflush");
	}
}


static
void send_head_escaped(
	FILE                       *sockf,
	const struct hyperfs_state *remote,
	const char                 *path,
	int                         n)
{
	char buf[4096];  // try to avoid allocations for the most part
	char *p = n < sizeof buf ? buf : malloc(n + 1);

	LOG("[send_head_escaped: escaping path to size %d]\n", n);

	escape_raw(p, path);
	send_head_plain(sockf, remote, p);

	if (p != buf)
		free(p);
}



static
int get_head_info(
	const  struct hyperfs_state *remote,
	const  char                 *path,
	struct resp_info            *resp)
{
	int sock = tcp_connect(remote->host, remote->port);
	if (sock < 0) {
		perror("tcp_connect");
		LOG("[get_head_info: tcp_connect returned %d]\n", sock);
		return -1;
	}
	FILE *sockf = fdopen(sock, "r+");
	LOG("[get_http_path_info: sock is on %d]\n", sock);

	if (!sockf) {
		LOG("[fdopen: %s]\n", strerror(errno));
		return -1;
	}

	int n = path_needs_escape(path);
	if (n) {
		send_head_escaped(sockf, remote, path, n);
	} else {
		send_head_plain(sockf, remote, path);
	}

	int ret = get_resp_info(sockf, resp);
	fclose(sockf);  // TODO: leave open
	return ret;
}

static
char *pfxmatch(const char *pfx, const char *s)
{
	size_t len = strlen(pfx);
	return strncmp(pfx, s, strlen(pfx)) == 0 ? (char *) s + len : NULL;
}

static
const char *skip_host(const char *location, const struct hyperfs_state *remote)
{
	if (!(location = pfxmatch(remote->proto, location))) return NULL;
	if (!(location = pfxmatch("://",         location))) return NULL;
	if (!(location = pfxmatch(remote->host,  location))) return NULL;

	if (location[0] == ':') {
		location++;
		if (!(location = pfxmatch(remote->port, location)))
			return NULL;
	}
	return location[0] == '/' ? location : NULL;
}


static
int location_adds_slash(
	const char *path,
	const char *location,
	const struct hyperfs_state *remote)
{
	if (strstr(location, "://")) {
		location = skip_host(location, remote);
		if (!location)
			return 0;
	}
	return (location = pfxmatch(remote->rootpath, location))
	    && (location = pfxmatch(path, location))
	    && strcmp(location, "/") == 0;
}


static inline
uint64_t nonneg(off_t n) { return n < 0 ? 0 : n; }

int get_http_path_info(
	const  char     *path,
	struct ministat *info)
{

	if (strcmp(path, "/") == 0) {
		info->size  = 0;
		info->mtime = 0;
		info->type  = S_IFDIR;
		return 0;
	}

	struct hyperfs_state *remote = get_hyperfs_state();

	struct resp_info resp;

	int ret = get_head_info(remote, path, &resp);

	if (ret < 0) {
		LOG("[get_http_path_info: get_resp_info returned %d]\n", ret);
		return -EIO;  // XXX: stat(1) does not set EIO
	}
	LOG("[get_http_path_info: response code was %d]\n", resp.code);
	if (resp.code == 301 || resp.code == 302) {
		// Moved Permanently, or Found; follow Location header
		LOG("[get_http_path_info: Location: %s]\n", resp.location);
		if (location_adds_slash(path, resp.location, remote)) {
			// fine, it's just a dir
			info->size  = 0;
			info->mtime = 0;
			info->type  = S_IFDIR;
		} else {
			// be extra nice and treat this like a symlink;
			// we aren't going to follow it ourselves though
			info->size  = strlen(resp.location);
			info->link  = addpath(resp.location);
			info->type  = S_IFLNK;
		}
	} else if (200 <= resp.code && resp.code <= 299) {
		info->size  = nonneg(resp.content_length);
		info->mtime = resp.last_modified;
		info->type  = S_IFREG;
	} else if (resp.code == 404) {
		return -ENOENT;
	} else {
		LOG("[get_http_path_info: got %d for: %s]\n", resp.code, path);
		return -EREMOTEIO;  // XXX: stat(1) does not set this code
	}

	return 0;
}


int hyperfs_getattr(const char *path, struct stat *stbuf)
{
	LOG("[getattr: '%s']\n", path);

	struct ministat mst;
	if (get_cached_path_info(path, &mst) < 0) {

		int ret = get_http_path_info(path, &mst);
		if (ret < 0)
			return ret;

		set_cached_path_info(path, &mst);  // ignore failure
	}
	expand_ministat(&mst, stbuf);

	return 0;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

