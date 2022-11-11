#pragma once

#include <stdio.h>  // FILE


FILE *getrange(
	const char *host,
	const char *port,
	const char *path,
	off_t start,
	size_t *len,
	char *buf);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

