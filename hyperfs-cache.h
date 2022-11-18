#pragma once

struct ministat;
struct stat;

void init_cache();
void free_cache();
int get_cached_path_info(const char *path, struct ministat *st);
int set_cached_path_info(const char *path, const struct ministat *st);
void expand_ministat(const struct ministat *mst, struct stat *st);
char *addpath(const char *path);
char *get_pathbuf(size_t size);
char *add_pathbuf(const char *buf, size_t size);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

