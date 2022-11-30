#pragma once

#include <stdio.h>  // FILE

struct hyperfs_state;

FILE *hyperget(
	struct hyperfs_state *remote,
	const char           *path,
	off_t                *content_len);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

