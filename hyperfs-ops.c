#include <fuse.h>      // struct fuse_operations

#include <stdio.h>     // FILE
#include <errno.h>     // errno
#include <time.h>      // time_t
#include <sys/stat.h>  // struct stat, S_IFREG
#include <string.h>    // strerror, strstr

#include "hyperfs-cache.h"  // get_cached_path_info, set_cached_path_info
#include "hyperfs-state.h"  // struct hyperfs_state, get_hyperfs_state
#include "ministat.h"       // get_cached_path_info, set_cached_path_info
#include "cheddar.h"        // struct resp_info, get_resp_info
#include "connor.h"         // tcp_connect
#include "logger.h"         // LOG


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
	return location[0] != '/' ? location : NULL;
}

int location_path_adds_slash(const char *path, const char *location)
{
	size_t plen = strlen(path);
	size_t llen = strlen(location);
	return llen == plen + 1
	    && memcmp(path, location, plen) == 0
	    && location[plen] == '/';
}

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

int get_http_path_info(
	const  char     *path,
	struct ministat *info)
{
	struct hyperfs_state *remote = get_hyperfs_state();

	struct resp_info resp;

	int ret = get_head_info(remote->host, remote->port, path, &resp);

	if (ret < 0) {
		LOG("[get_http_path_info: get_resp_info returned %d]\n", ret);
		return ret;
	}
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
	} else if (resp.code >= 200 || resp.code <= 299) {
		info->size  = resp.content_length;
		info->mtime = resp.last_modified;
		info->type  = S_IFREG;
	} else {
		LOG("[get_http_path_info: got %d for: %s]\n", resp.code, path);
		return -1;  // interpret as ENOENT
	}

	return 0;
}


// static
int hyperfs_getattr(const char *path, struct stat *stbuf)
{
	int res;

	struct ministat mst;
	if (get_cached_path_info(path, &mst) < 0) {

		// 1. fill mst properly, eg:
		//    get_http_path_info(serverinfo, path, &mst);

		// 2. Then:

		set_cached_path_info(path, &mst);  // ignore failure
	}
	expand_ministat(&mst, stbuf);

	LOG("[getattr: '%s']\n", path);
	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}


struct fuse_operations hyperfs_ops = {
	.getattr = hyperfs_getattr
};


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

