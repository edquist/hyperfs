#include <stdio.h>   // printf

#include "escape.h"  // decode_escaped


void print_decoded(char *s)
{
	decode_escaped(s, s);
	printf("%s\n", s);
}

int main(int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++)
		print_decoded(argv[i]);

	return 0;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

