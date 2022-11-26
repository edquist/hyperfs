
#include "hyperfs-state.h"
#include "connor.h"  // get_tcp_addrinfo, connect_first
#include "loggo.h"   // LOG


int hyperconnect(struct hyperfs_state *remote)
{
	if (remote->sockf) {
		fclose(remote->sockf);
		remote->sockf = NULL;
	}
	if (!remote->ainfo) {
		remote->ainfo = get_tcp_addrinfo(remote->host, remote->port);
		if (!remote->ainfo) {
			LOG("[hyperconnect: get_tcp_addrinfo failed]\n");
			perror("get_tcp_addrinfo");
			return -1;
		}
	}

	int sock = connect_first(remote->ainfo);
	if (sock == -1) {
		LOG("[hyperconnect: could not connect]\n");
		perror("connect_first");
		return -1;
	} else {
		LOG("[hyperconnect: sock is on %d]\n", sock);
	}

	remote->sockf = fdopen(sock, "r+");
	if (!remote->sockf) {
		LOG("[hyperconnect: fdopen failed]\n");
		perror("fdopen");
		return -1;
	}

	return 0;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

