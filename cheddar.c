#include <stdio.h>
#include <stdlib.h>  // atoi
#include <string.h>  // strlen
#include <strings.h> // strncasecmp
#include <time.h>    // time_t

#include "date_parse.h"

struct resp_info {
	int code;
	off_t content_length;
	time_t last_modified;
	// char server[64]
	// char content_type[80]
};

static
char *pfxmatch(const char *pfx, const char *s)
{
	size_t len = strlen(pfx);
	return strncasecmp(pfx, s, strlen(pfx)) == 0 ? (char *) s + len : NULL;
}

static
int get_resp_info(FILE *in, struct resp_info *info)
{
	char buf[128], *p;
	info->content_length = 0;
	if (!fgets(buf, sizeof buf, in))         return -1;
	if (sscanf(buf, "%*s %d ", &info->code) != 1) return -2;
	while (fgets(buf, sizeof buf, in)) {
		if (buf[0] == '\r') {
			return info->content_length ? 0 : -3;
		}
		if ((p = pfxmatch("Content-Length: ", buf))) {
			info->content_length = atoi(p);
		} else if ((p = pfxmatch("Last-Modified: ", buf))) {
			info->last_modified = date_parse(p);
		}
	}

	return -4;
}

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


int get_resp_data(FILE *in, char *dest, size_t *len)
{
	struct resp_info info;
	int ret = get_resp_info(in, &info);
	if (ret < 0) {
		fprintf(stderr, "get_resp_info returned %d\n", ret);
		return ret;
	}
	fprintf(stderr, "code %d\n", info.code);
	if (info.code < 200 || info.code > 299)
		return -info.code;

	if (info.content_length != *len) {
		fprintf(stderr,
			"asked for %zu bytes; got content-length %zu\n",
			*len, info.content_length);

		if (info.content_length > *len) {
			fprintf(stderr, "more than we asked for!\n");
			return -10;
		}
	}

	//size_t sent = pump(in, out, info.content_length);
	
	*len = fread(dest, 1, info.content_length, in);

	if (info.content_length != *len) {
		fprintf(stderr, "got short fread count\n");
	}

	return 0;
}



/* vim: set noexpandtab sts=0 sw=8 ts=8: */

