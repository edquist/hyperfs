#pragma once

#include <stdio.h>   // FILE
#include <stddef.h>  // size_t
#include <time.h>    // time_t

struct resp_info {
	int code;
	off_t content_length;
	time_t last_modified;
	char location[1024];
};

char *pfxmatch(const char *pfx, const char *s);
char *pfxcasematch(const char *pfx, const char *s);
int get_resp_data(FILE *in, char *dest, size_t *len);
int get_resp_info(FILE *in, struct resp_info *info);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

