#pragma once

#include <stdio.h>   // FILE
#include <stddef.h>  // size_t

int get_resp_data(FILE *in, char *dest, size_t *len);
