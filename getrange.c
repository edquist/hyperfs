#include <stdio.h>  // printf, FILE
#include <stdlib.h> // malloc, free
#include <errno.h>  // perror

#include "connor.h"  // tcp_connect
#include "cheddar.h" // get_resp_data
#include "sendo.h"   // SENDO
#include "escape.h"  // path_needs_escape, escape_raw

static
void send_get_plain(FILE *sockf, const char *root, const char *path)
{
	SENDO(sockf, "GET %s%s HTTP/1.1", root, path);
}


static
void send_get_escaped(FILE *sockf, const char *root, const char *path, int n)
{
	char buf[4096];  // try to avoid allocations for the most part
	char *p = n < sizeof buf ? buf : malloc(n + 1);

	LOG("[send_get_escaped: escaping path to size %d]\n", n);

	escape_raw(p, path);
	send_get_plain(sockf, root, p);

	if (p != buf)
		free(p);
}


FILE *getrange(
	const char *host,
	const char *port,
	const char *rootpath,
	const char *path,
	off_t start,
	size_t *len,
	char *buf)
{
	int sock = tcp_connect(host, port);
	if (sock < 0)
		return NULL;

	FILE *sockf = fdopen(sock, "r+");
	off_t end = start + *len - 1;

	fprintf(stderr, "sock is on %d\n", sock);

	if (!sockf) {
		perror("fdopen");
		return NULL;
	}

	int n = path_needs_escape(path);
	if (n) {
		send_get_escaped(sockf, rootpath, path, n);
	} else {
		send_get_plain(sockf, rootpath, path);
	}
	SENDO(sockf, "Host: %s", host);
	// SENDO(sockf, "Accept: */*");
	SENDO(sockf, "User-Agent: hyperfs");
	SENDO(sockf, "Range: bytes=%zu-%zu", start, end);
	SENDO(sockf, "");

	if (fflush(sockf)) {
		perror("fflush");
		return NULL;
	}

	int ret = get_resp_data(sockf, buf, len);
	if (ret < 0) {
		fprintf(stderr, "get_resp_data returned %d\n", ret);
		return NULL;
	}

	return sockf;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

