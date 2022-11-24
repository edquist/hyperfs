#include <stdio.h>   // fprintf, FILE, perror
#include <stdlib.h>  // malloc, free  :(
#include <unistd.h>  // close

#include "hyperfs-state.h"  // struct hyperfs_state
#include "connor.h"         // tcp_connect
#include "cheddar.h"        // struct resp_info
#include "escape.h"         // path_needs_escape, escape_raw
#include "loggo.h"          // LOG
#include "sendo.h"          // SENDO


static
void send_get_plain(
	FILE                 *sockf,
	struct hyperfs_state *remote,
	const char           *path)
{
	if (path[0] == '/' && path[1] == '\0')
		path++;  // avoid double-slash for fs root

	SENDO(sockf, "GET %s%s/ HTTP/1.1", remote->rootpath, path);
	SENDO(sockf, "Host: %s", remote->host);  // skip port
	// SENDO(sockf, "Accept: */*");
	// SENDO(sockf, "User-Agent: hyperfs");
	S_ENDO(sockf);

	if (fflush(sockf)) {
		LOG("[send_get_plain: fflush failed; is connection closed?]\n");
		perror("fflush");
	}
}


static
void send_get_escaped(
	FILE                 *sockf,
	struct hyperfs_state *remote,
	const char           *path,
	int                   n)
{
	char buf[4096];  // try to avoid allocations for the most part
	char *p = n < sizeof buf ? buf : malloc(n + 1);

	LOG("[send_get_escaped: escaping path to size %d]\n", n);

	escape_raw(p, path);
	send_get_plain(sockf, remote, p);

	if (p != buf)
		free(p);
}


FILE *hyperget(
	struct hyperfs_state *remote,
	const char           *path,
	size_t               *content_len)
{
	LOG("[hyperget: '%s']\n", path);

	int sock = tcp_connect(remote->host, remote->port);
	if (sock < 0) {
		perror("tcp_connect");
		LOG("[hyperget: tcp_connect returned %d]\n", sock);
		return NULL;
	}

	FILE *sockf = fdopen(sock, "r+");
	if (!sockf) {
		perror("fdopen");
		LOG("[hyperget: fdopen failed]\n");
		close(sock);
		return NULL;
	}

	int n = path_needs_escape(path);
	if (n) {
		send_get_escaped(sockf, remote, path, n);
	} else {
		send_get_plain(sockf, remote, path);
	}

	struct resp_info resp;
        int ret = get_resp_info(sockf, &resp);
	if (ret < 0) {
		LOG("[hyperget: get_resp_info returned %d]\n", ret);
		fclose(sockf);
		return NULL;
	}

	LOG("[hyperget: got code %d]\n", resp.code);
	if (resp.code < 200 || 299 < resp.code) {
		fclose(sockf);
		return NULL;
	}

	*content_len = resp.content_length;
	return sockf;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

