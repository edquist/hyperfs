#include <stdio.h>  // printf, FILE
#include <stdlib.h> // exit
#include <errno.h>  // perror

#include "connor.h"  // tcp_connect
#include "cheddar.h" // get_resp_data

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
		exit(1);
	}

	fprintf(sockf,
		"GET %s HTTP/1.1\r\n"
		"Host: %s:%s\r\n"  // XXX: service needs to be numeric
		// "Accept: */*\r\n"
		// "User-Agent: hyperfs\r\n"
		"Range: bytes=%zu-%zu\r\n"
		"\r\n", path, host, port, start, end);

	fflush(sockf); // XXX: check

	// and now read...

	int ret = get_resp_data(sockf, buf, len);
	if (ret < 0) {
		fprintf(stderr, "get_resp_data returned %d\n", ret);
		exit(1);
	}

	return sockf;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

