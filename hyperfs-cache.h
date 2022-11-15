#pragma once

#include <stdint.h>    // uint64_t

struct ministat {
	uint64_t size;
	time_t mtime;
	int mode;
};

void init_cache();
void free_cache();
int get_cached_path_info(const char *path, struct ministat *st);
int set_cached_path_info(const char *path, const struct ministat *st);

