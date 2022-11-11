#include <stdio.h>  // printf
#include <stdlib.h> // exit
#include <errno.h>  // perror

#include "getrange.h"

void usage(const char *prog)
{
	fprintf(stderr, "usage: %s host port path start end\n", prog);
	exit(0);
}


int main(int argc, char **argv, char **envp)
{
	const char *host, *port, *path;
	char *buf;
	off_t start, end;
	size_t len;

	if (argc != 6)
		usage(argv[0]);

	host  = argv[1];
	port  = argv[2];
	path  = argv[3];
	start = atoll(argv[4]);
	end   = atoll(argv[5]);
	len   = end - start + 1;

	buf = malloc(len);

	FILE *sockf = getrange(host, port, path, start, &len, buf);

	if (!sockf) {
		perror("getrange");
		return 1;
	}

	fwrite(buf, 1, len, stdout);  // XXX: check
	fflush(stdout);               // XXX: check

	free(buf);
	fclose(sockf);

	return 0;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

