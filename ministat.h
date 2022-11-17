#pragma once

#include <stdint.h>    // uint64_t


struct ministat {
	uint64_t size;
	union {
		time_t  mtime;
		char   *link;   // link target for type == S_IFLNK
	};
	int type;
};


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

