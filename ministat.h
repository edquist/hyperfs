#pragma once

#include <stdint.h>    // uint64_t
#include <time.h>      // time_t   ... oof.


struct ministat {
	union {
		uint64_t size;
		char *hyperdir;  // dir listing for type == S_IFDIR
	};
	union {
		time_t  mtime;
		char   *link;   // link target for type == S_IFLNK
	};
	int type;
};


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

