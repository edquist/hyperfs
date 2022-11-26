#pragma once

#include <sys/types.h>  // off_t, size_t

struct hyperfs_state;

int hyperget_range(
	struct hyperfs_state *remote,
	const char *path,
	off_t start,
	size_t len,
	char *buf);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

