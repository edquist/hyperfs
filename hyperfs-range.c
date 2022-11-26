#include <stdio.h>  // printf, FILE
#include <stdlib.h> // malloc, free
#include <errno.h>  // perror

#include "cheddar.h" // get_resp_data
#include "sendo.h"   // SENDO
#include "escape.h"  // path_needs_escape, escape_raw

#include "hyperfs-state.h"       // struct hyperfs_state
#include "hyperfs-connect.h"     // hyperconnect, hyperclose


static
void send_get_plain(struct hyperfs_state *remote, const char *path)
{
	SENDO(remote->sockf, "GET %s%s HTTP/1.1", remote->rootpath, path);
}


static
void send_get_escaped(struct hyperfs_state *remote, const char *path, int n)
{
	char buf[4096];  // try to avoid allocations for the most part
	char *p = n < sizeof buf ? buf : malloc(n + 1);

	LOG("[send_get_escaped: escaping path to size %d]\n", n);

	escape_raw(p, path);
	send_get_plain(remote, p);

	if (p != buf)
		free(p);
}


int hyperget_range(
	struct hyperfs_state *remote,
	const char           *path,
	off_t                 start,
	size_t                len,
	char                 *buf)
{
	LOG("[hyperget_range: '%s' start=%ld len=%zu]\n", path, start, len);

	if (len == 0)
		return 0;

	if (!remote->sockf && hyperconnect(remote) < 0) {
		LOG("[hyperget_range: hyperconnect failed]\n");
		return -1;
	}

	off_t end = start + len - 1;

	int n = path_needs_escape(path);
	if (n) {
		send_get_escaped(remote, path, n);
	} else {
		send_get_plain(remote, path);
	}
	SENDO(remote->sockf, "Host: %s", remote->host);
	SENDO(remote->sockf, "User-Agent: hyperfs");
	SENDO(remote->sockf, "Range: bytes=%zu-%zu", start, end);
	SENDO(remote->sockf, "");

	if (fflush(remote->sockf)) {
		perror("fflush");
		hyperclose(remote);
		return -1;
	}

	int ret = get_resp_data(remote->sockf, buf, &len);
	if (ret < 0) {
		LOG("[hyperget_range: get_resp_data returned %d]\n", ret);
		hyperclose(remote);
		return ret;
	}

	return len;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

