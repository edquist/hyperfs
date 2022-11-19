#pragma once

#include <stdint.h>   // uint64_t


struct ministat;

union hyperfs_finfo {
	uint64_t fh;
	const struct ministat *ms;
};


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

