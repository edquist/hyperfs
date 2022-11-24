#define  _GNU_SOURCE  // asprintf

#include <fuse.h>   // fuse_main

#include <stdio.h>  // printf, perror
#include <stdlib.h> // exit
#include <string.h> // strlen, strcpy, strstr
#include <regex.h>  // regcomp, regexec, regfree

#include "hyperfs-ops.h"    // hyperfs_ops
#include "hyperfs-cache.h"  // init_cache, free_cache
#include "hyperfs-state.h"  // struct hyperfs_state
#include "loggo.h"          // init_logger, LOG
#include "xasprintf.h"      // xasprintf

static
void usage(const char *prog)
{
	printf("usage: %s [http://]host[:port][/path/] mountpoint "
	       " [fuse-options]\n", prog);
	exit(0);
}

static
void chompc(char *s, char c)
{
	size_t len = strlen(s);
	if (s[len - 1] == c)
		s[len - 1] = 0;
}

void rematch_dump(const char *s, const regmatch_t *m, int n)
{
	int i;
	for (i = 0; i < 7; i++) {
		printf("m[%d] = [%2d:%2d] '%.*s'\n",
		       i, m[i].rm_so, m[i].rm_eo,
		       m[i].rm_eo - m[i].rm_so,
		       m[i].rm_so < 0 ? "" : s + m[i].rm_so);
	}
}

char *dupmatch(const char *s, const regmatch_t *m)
{
	return m->rm_so < 0 ? NULL
	                    : strndup(s + m->rm_so, m->rm_eo - m->rm_so);
}

char *dupmatch2(const char *s, const regmatch_t *m, const char *dflt)
{
	char *d = dupmatch(s, m);
	return d ? d : strdup(dflt);
}

static
char *parse_url(const char *url, struct hyperfs_state *state)
{
	regex_t reg;
	regmatch_t m[7];
	const char *re = "^(([a-z]+)://)?([^:/]+)(:([0-9]+))?(/.*)?$";
	int ret = regcomp(&reg, re, REG_EXTENDED);
	if (ret != 0) {
		perror("regcomp");
		exit(1);
	}
	ret = regexec(&reg, url, 7, m, 0);
	regfree(&reg);
	if (ret != 0)
		FAIL("failed to parse URL\n");

	state->proto    = dupmatch2(url, &m[2], "http");
	state->host     = dupmatch (url, &m[3]);
	state->port     = dupmatch2(url, &m[5], "80");
	state->rootpath = dupmatch2(url, &m[6], "");

	chompc(state->rootpath, '/');

	if (strcmp(state->proto, "http") != 0)
		FAIL("Sorry, only http is supported\n");

	char *url_norm = xasprintf("%s://%s:%s%s/", state->proto, state->host,
	                                        state->port, state->rootpath);

	return url_norm;
}


// src is NULL terminated; we do not NULL terminate dest though
static
char **copy_args(char **dest, char *const *src)
{
	while (*src)
		*dest++ = *src++;
	return dest;
}

int main(int argc, char **argv, char **envp)
{
	int res;
	struct hyperfs_state state = {};

	if (argc < 3 || 60 < argc)
		usage(argv[0]);

	char *fuse_args[64], **fuse_argp = fuse_args;

	char *url = parse_url(argv[1], &state);
	char *mountpoint = argv[2];
	char *fsname_opt = xasprintf("-ofsname=%s", url);

	*fuse_argp++ = argv[0];
	// skip argv[1] (URL), which we add later as fsname_opt
	fuse_argp    = copy_args(fuse_argp, &argv[2]);

	*fuse_argp++ = "-s";        // single-threaded fs, please
	*fuse_argp++ = fsname_opt;
	*fuse_argp   = NULL;
	int fuse_argc = fuse_argp - fuse_args;

	init_logger();
	LOG("[hyperfs: Greets!]\n");
	LOG("[hyperfs: mounting <%s> onto '%s']\n", url, mountpoint);
	init_cache();
	res = fuse_main(fuse_argc, fuse_args, &hyperfs_ops, &state);

	LOG("[hyperfs: Good-bye!]\n");

	// cleanup as necessary
	free(fsname_opt);
	free_cache();

	return res;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

