#include <stdio.h>

size_t drainf(FILE *in, size_t len)
{
	char buf[8 * 1024];
	size_t rem = len;
	while (rem) {
		size_t n = rem > sizeof buf ? sizeof buf : rem;
		size_t got = fread(buf, 1, n, in);
		rem -= got;
		if (got < n) {
			// ferror(in);
			fprintf(stderr, "unexpected short read count\n");
			break;
		}
	}

	return len - rem;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

