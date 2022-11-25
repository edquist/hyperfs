#include <stdio.h>  // printf, FILE
#include <errno.h>  // perror

#include "connor.h"  // tcp_connect
#include "cheddar.h" // get_resp_data
#include "sendo.h"   // SENDO

FILE *getrange(
	const char *host,
	const char *port,
	const char *path,
	off_t start,
	size_t *len,
	char *buf)
{
	int sock = tcp_connect(host, port);

	FILE *sockf = fdopen(sock, "r+");
	off_t end = start + *len - 1;

	fprintf(stderr, "sock is on %d\n", sock);

	if (!sockf) {
		perror("fdopen");
		return NULL;
	}

	SENDO(sockf, "GET %s HTTP/1.1", path);
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

