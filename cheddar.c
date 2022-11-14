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
	char location[1024];
};

static
char *pfxmatch(const char *pfx, const char *s)
{
	size_t len = strlen(pfx);
	return strncasecmp(pfx, s, strlen(pfx)) == 0 ? (char *) s + len : NULL;
}

static
size_t hdrvalcpy(char *dest, const char *src, size_t dstsize)
{
	size_t len = strlen(src);
	if (len >= 2 && src[len - 1] == '\n' && src[len - 2] == '\r')
		len -= 2;
	if (len >= dstsize)
		len = dstsize - 1;
	memcpy(dest, src, len);
	dest[len] = '\0';
	return len;
}

static
int get_resp_info(FILE *in, struct resp_info *info)
{
	char buf[1024], *p;
	info->content_length = 0;
	info->last_modified = 0;
	info->location[0] = 0;
	if (!fgets(buf, sizeof buf, in))              return -1;
	if (sscanf(buf, "%*s %d ", &info->code) != 1) return -2;
	while (fgets(buf, sizeof buf, in)) {
		if (buf[0] == '\r') {
			return info->content_length ? 0 : -3;
		}
		// if (debug) fprintf(stderr, "< %s", buf);
		if ((p = pfxmatch("Content-Length: ", buf))) {
			info->content_length = atoi(p);
		} else if ((p = pfxmatch("Last-Modified: ", buf))) {
			info->last_modified = date_parse(p);
		} else if ((p = pfxmatch("Location: ", buf))) {
			hdrvalcpy(info->location, p, sizeof info->location);
		}
	}

	return -4;
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
	if (info.code == 301 || info.code == 302) {
		// Moved Permanently, or Found; follow Location header

	}

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

	*len = fread(dest, 1, info.content_length, in);

	if (info.content_length != *len) {
		fprintf(stderr, "got short fread count\n");
	}

	return 0;
}



/* vim: set noexpandtab sts=0 sw=8 ts=8: */

