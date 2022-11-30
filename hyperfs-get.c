#include <stdio.h>   // fprintf, FILE, perror
#include <stdlib.h>  // malloc, free  :(
#include <unistd.h>  // close

#include "hyperfs-connect.h" // hyperconnect
#include "hyperfs-state.h"  // struct hyperfs_state
#include "connor.h"         // tcp_connect
#include "cheddar.h"        // struct resp_info
#include "escape.h"         // path_needs_escape, escape_raw
#include "loggo.h"          // LOG
#include "sendo.h"          // SENDO


static
void send_get_plain(
	struct hyperfs_state *remote,
	const char           *path)
{
	if (path[0] == '/' && path[1] == '\0')
		path++;  // avoid double-slash for fs root

	FILE *sockf = remote->sockf;
	SENDO(sockf, "GET %s%s/ HTTP/1.1", remote->rootpath, path);
	SENDO(sockf, "Host: %s", remote->host);  // skip port
	SENDO(sockf, "User-Agent: hyperfs");
	SENDO(sockf, "Connection: keep-alive");
	SENDO(sockf, "");

	if (fflush(sockf)) {
		LOG("[send_get_plain: fflush failed; is conn closed?]\n");
		perror("fflush");
	}
}


static
void send_get_escaped(
	struct hyperfs_state *remote,
	const char           *path,
	int                   n)
{
	char buf[4096];  // try to avoid allocations for the most part
	char *p = n < sizeof buf ? buf : malloc(n + 1);

	LOG("[send_get_escaped: escaping path to size %d]\n", n);

	escape_raw(p, path);
	send_get_plain(remote, p);

	if (p != buf)
		free(p);
}



static
int get_index_info(
	struct hyperfs_state *remote,
	const  char          *path,
	struct resp_info     *resp)
{
	int n = path_needs_escape(path);
	if (n) {
		send_get_escaped(remote, path, n);
	} else {
		send_get_plain(remote, path);
	}

	int ret = get_resp_info(remote->sockf, resp);
	if (ret < 0)
		LOG("[get_index_info: get_resp_info returned %d]\n", ret);
	return ret;
}


static
int get_index_info2x(
	struct hyperfs_state *remote,
	const  char          *path,
	struct resp_info     *resp)
{
	int ret = get_index_info(remote, path, resp);
	if (ret < 0) {
		LOG("[hyperget: reconnecting to retry get_index_info]\n");
		if (hyperconnect(remote) < 0)
			return -1;
		ret = get_index_info(remote, path, resp);
	}
	return ret;
}


FILE *hyperget(
	struct hyperfs_state *remote,
	const char           *path,
	off_t                *content_len)
{
	LOG("[hyperget: '%s']\n", path);

	if (!remote->sockf && hyperconnect(remote) < 0) {
		LOG("[hyperget: hyperconnect failed]\n");
		return NULL;
	}

	struct resp_info resp;
	int ret = get_index_info2x(remote, path, &resp);
	if (ret < 0) {
		LOG("[hyperget: get_resp_info returned %d]\n", ret);
		hyperclose(remote);
		return NULL;
	}

	LOG("[hyperget: got code %d]\n", resp.code);
	if (resp.code < 200 || 299 < resp.code) {
		return NULL;
	}

	*content_len = resp.content_length;
	return remote->sockf;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

