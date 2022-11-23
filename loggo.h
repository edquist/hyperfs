#pragma once

#include <stdio.h>  // FILE, fprintf


#define LOG(...) do { if (logger) fprintf(logger, __VA_ARGS__); } while (0)


extern FILE *logger;

void init_logger();



/* vim: set noexpandtab sts=0 sw=8 ts=8: */

