#include <stdio.h>   // printf

#include "escape.h"  // path_needs_escape, escape_raw


void print_escaped(const char *s)
{
	char buf[4096];

	int n = path_needs_escape(s);
	if (n == 0) {
		printf("%s\n", s);
	} else if (n < sizeof buf) {
		escape_raw(buf, s);
		printf("%s\n", buf);
	} else {
		fprintf(stderr, "c'mon man...\n");
	}
}

int main(int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++)
		print_escaped(argv[i]);

	return 0;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

