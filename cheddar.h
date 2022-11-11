#pragma once

#include <stdio.h>   // FILE
#include <stddef.h>  // size_t

size_t pump(FILE *in, FILE *out, size_t len);  // unused

int get_resp_data(FILE *in, char *dest, size_t *len);
