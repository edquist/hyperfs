#pragma once

#ifndef _GNU_SOURCE
#error Must define _GNU_SOURCE for asprintf/xasprintf
#endif

#include <stdio.h>    // asprintf

#include "fail.h"     // FAIL


// do this as a macro to ease passing __VA_ARGS__

#define xasprintf(...) \
	({ \
		char *s; \
		if (asprintf(&s, __VA_ARGS__) < 0) FAIL("asprintf alloc"); \
		s; \
	}) \


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

