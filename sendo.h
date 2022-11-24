#pragma once

#include "loggo.h"   // LOG


#define SENDO(_sock, _fmt, ...) \
	do { LOG("< " _fmt "\n", __VA_ARGS__); \
	     fprintf(_sock, _fmt "\r\n", __VA_ARGS__); } while (0)


// kind of a hack; but we need a format string + something for __VA_ARGS__
#define S_ENDO(_sock) SENDO(_sock, "%s", "")


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

