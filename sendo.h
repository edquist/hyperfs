#pragma once

#include "loggo.h"   // LOG


#define SENDO(_sock, ...) \
	do { LOG("< "); LOG(__VA_ARGS__); LOG("\n"); \
	     fprintf(_sock, __VA_ARGS__); fprintf(_sock, "\r\n"); } while (0)


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

