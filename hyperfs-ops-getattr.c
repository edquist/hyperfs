#include <stdio.h>     // FILE
#include <errno.h>     // errno
#include <time.h>      // time_t
#include <sys/stat.h>  // S_IFREG, S_IFDIR, S_IFLNK
#include <string.h>    // strerror, strstr

#include "hyperfs-cache.h"  // get_cached_path_info, set_cached_path_info
#include "hyperfs-state.h"  // struct hyperfs_state, get_hyperfs_state
#include "ministat.h"       // get_cached_path_info, set_cached_path_info
#include "cheddar.h"        // struct resp_info, get_resp_info
#include "connor.h"         // tcp_connect
#include "logger.h"         // LOG


static
int get_head_info(
	const  char          *host,
	const  char          *port,
	const  char          *path,
	struct resp_info     *resp)
{
	int sock = tcp_connect(host, port);
	FILE *sockf = fdopen(sock, "r+");
	LOG("[get_http_path_info: sock is on %d]\n", sock);

	if (!sockf) {
		LOG("[fdopen: %s]\n", strerror(errno));
		return -1;
	}

	fprintf(sockf,
		"HEAD %s HTTP/1.1\r\n"
		"Host: %s:%s\r\n"        // XXX: service needs to be numeric
		"Connection: close\r\n"
		"\r\n", path, host, port);
	fflush(sockf);  // XXX: check

	int ret = get_resp_info(sockf, resp);
	fclose(sockf);  // TODO: leave open
	return ret;
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
int location_path_adds_slash(const char *path, const char *location)
{
	size_t plen = strlen(path);
	size_t llen = strlen(location);
	return llen == plen + 1
	    && memcmp(path, location, plen) == 0
	    && location[plen] == '/';
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
	return location_path_adds_slash(path, location);
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

	int ret = get_head_info(remote->host, remote->port, path, &resp);

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
	} else if (resp.code >= 200 && resp.code <= 299) {
		info->size  = nonneg(resp.content_length);
		info->mtime = resp.last_modified;
		info->type  = S_IFREG;
	} else if (resp.code == 404) {
		return -ENOENT;
	} else {
		LOG("[get_http_path_info: got %d for: %s]\n", resp.code, path);
		return -EIO;  // XXX: stat(1) does not set EIO
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

