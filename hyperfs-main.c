#include <fuse.h>   // fuse_main

#include <stdio.h>  // printf, perror
#include <stdlib.h> // exit
#include <string.h> // strlen, strcpy

#include "hyperfs-ops.h"    // hyperfs_ops
#include "hyperfs-cache.h"  // init_cache, free_cache
#include "hyperfs-state.h"  // struct hyperfs_state
#include "logger.h"         // init_logger

static
void usage(const char *prog)
{
	printf("usage: %s [http://]host[:port][/path/] mountpoint "
	       " [fuse-options]\n", prog);
	exit(0);
}

static
void parse_url(const char *url, struct hyperfs_state *state)
{
	state->proto    = "http";  // we insist...
	state->host     = url;     // TODO: parse
	state->port     = "80";
	state->port_num = 80;
	state->sockfd   = -1;
	state->sockf    = NULL;
}


static
void shift_n_push(int i, int argc, /*const*/ char **argv, /*const*/ char *item)
{
	for (; i < argc - 1; i++)
		argv[i] = argv[i + 1];
	argv[argc - 1] = item;
}

static
char *get_fsname_opt(const char *url)
{
	static const char *opt = "-ofsname=";
	size_t len = strlen(opt) + strlen(url);
	char *buf = malloc(len + 1);
	if (!buf) {
		perror("malloc");
		exit(1);
	}
	strcpy(buf, opt);
	strcpy(buf + strlen(opt), url);
	return buf;
}


int main(int argc, char **argv, char **envp)
{
	int res;
	struct hyperfs_state state = {};

	if (argc < 3)
		usage(argv[0]);

	char *url = argv[1];
	// char *mountpoint = argv[2];  // we don't actually look at this

	parse_url(url, &state);

	// shift url off front of argv[1:], push fsname_opt onto the end
	char *fsname_opt = get_fsname_opt(url);
	shift_n_push(1, argc, argv, fsname_opt);

	init_logger();
	LOG("[hyperfs: Greets!]\n");
	init_cache();
	res = fuse_main(argc, argv, &hyperfs_ops, &state);

	LOG("[hyperfs: Good-bye!]\n");

	// cleanup as necessary
	free(fsname_opt);
	free_cache();

	return res;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

