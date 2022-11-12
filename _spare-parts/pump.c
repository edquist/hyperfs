
size_t pump(FILE *in, FILE *out, size_t len)
{
	char buf[8 * 1024];
	size_t rem = len;
	while (rem) {
		size_t n = rem > sizeof buf ? sizeof buf : rem;
		size_t got = fread(buf, 1, n, in);
		if (got < n) {
			// ferror(in);
			fprintf(stderr, "unexpected short read count\n");
		}

		size_t sent = fwrite(buf, 1, got, out);
		rem -= sent;

		if (sent < got) {
			// ferror(in);
			fprintf(stderr, "unexpected short write count\n");
		}

		if (sent < n)
			break;
	}

	return len - rem;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

