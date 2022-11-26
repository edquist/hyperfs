#pragma once


struct hyperfs_state;

void hyperclose(struct hyperfs_state *remote);
int hyperconnect(struct hyperfs_state *remote);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

