#define __ISOC99_SOURCE  // atoll

#include <stdio.h>
#include <stdlib.h>  // atoll
#include <string.h>  // strlen
#include <strings.h> // strncasecmp
#include <time.h>    // time_t

#include "date_parse.h"
#include "drainf.h"
#include "cheddar.h"
#include "loggo.h"

char *pfxcasematch(const char *pfx, const char *s)
{
	size_t len = strlen(pfx);
	return strncasecmp(pfx, s, strlen(pfx)) == 0 ? (char *) s + len : NULL;
}

static
size_t chomp_crlf(char *line)
{
	size_t len = strlen(line);
	if (len >= 2 && line[len - 1] == '\n' && line[len - 2] == '\r')
		line[len -= 2] = '\0';
	return len;
}

static
size_t hdrvalcpy(char *dest, const char *src, size_t dstsize)
{
	size_t len = strlen(src);
	if (len >= dstsize)
		len = dstsize - 1;
	memcpy(dest, src, len);
	dest[len] = '\0';
	return len;
}

int get_resp_info(FILE *in, struct resp_info *info)
{
	char buf[1024], *p;
	info->content_length = -1;
	info->last_modified = 0;
	info->location[0] = 0;
	if (!fgets(buf, sizeof buf, in))              return -1;
	chomp_crlf(buf);
	LOG("< %s\n", buf);
	if (sscanf(buf, "%*s %d ", &info->code) != 1) return -2;
	while (fgets(buf, sizeof buf, in)) {
		chomp_crlf(buf);
		LOG("< %s\n", buf);
		if (buf[0] == '\0')
			return 0;
		if ((p = pfxcasematch("Content-Length: ", buf))) {
			info->content_length = atoll(p);
		} else if ((p = pfxcasematch("Last-Modified: ", buf))) {
			info->last_modified = http_date_parse(p);
		} else if ((p = pfxcasematch("Location: ", buf))) {
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
		fprintf(stderr, "Location: %s\n", info.location);

	}
	if (info.code < 200 || info.code > 299) {
		if (info.content_length > 0)
			drainf(in, info.content_length);
		return -info.code;
	}

	if (info.content_length != *len) {
		fprintf(stderr,
			"asked for %zu bytes; got content-length %zu\n",
			*len, info.content_length);

		if (info.content_length > *len) {
			fprintf(stderr, "more than we asked for!\n");
			drainf(in, info.content_length);
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

