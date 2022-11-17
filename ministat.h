#pragma once

#include <stdint.h>    // uint64_t


struct ministat {
	uint64_t size;
	time_t mtime;
	int type;
};


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

