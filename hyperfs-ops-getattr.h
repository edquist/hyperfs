#pragma once

struct stat;

int hyperfs_getattr(const char *path, struct stat *stbuf);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

