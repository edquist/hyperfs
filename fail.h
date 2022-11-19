#pragma once

#include <stdio.h>    // fprintf

#define FAIL(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

