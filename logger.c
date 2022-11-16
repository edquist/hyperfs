#include <stdio.h>  // FILE, fdopen, stderr


FILE *logger;


void init_logger()
{
	if ((logger = fdopen(999, "w")) != NULL) {
		setbuf(logger, NULL);
		stderr = logger;
	}
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

