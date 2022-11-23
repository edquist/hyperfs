#include <stdio.h>   // fprintf, FILE, perror
#include <unistd.h>  // close

#include "hyperfs-state.h"  // struct hyperfs_state
#include "connor.h"         // tcp_connect
#include "cheddar.h"        // struct resp_info
#include "loggo.h"          // LOG


FILE *hyperget(
	struct hyperfs_state *remote,
	const char           *path,
	size_t               *content_len)
{
	LOG("[hyperget: '%s']\n", path);

	int sock = tcp_connect(remote->host, remote->port);
	if (sock < 0) {
		perror("tcp_connect");
		LOG("[hyperget: tcp_connect returned %d]\n", sock);
		return NULL;
	}

	FILE *sockf = fdopen(sock, "r+");
	if (!sockf) {
		perror("fdopen");
		LOG("[hyperget: fdopen failed]\n");
		close(sock);
		return NULL;
	}

	fprintf(sockf,  // XXX: check
		"GET %s/%s/ HTTP/1.1\r\n"
		"Host: %s\r\n"  // skip port
		// "Accept: */*\r\n"
		// "User-Agent: hyperfs\r\n"
		"\r\n", remote->rootpath, path + 1, remote->host);

	fflush(sockf);  // XXX: check

	struct resp_info resp;
        int ret = get_resp_info(sockf, &resp);
	if (ret < 0) {
		LOG("[hyperget: get_resp_info returned %d]\n", ret);
		fclose(sockf);
		return NULL;
	}

	LOG("[hyperget: got code %d]\n", resp.code);
	if (resp.code < 200 || 299 < resp.code) {
		fclose(sockf);
		return NULL;
	}

	*content_len = resp.content_length;
	return sockf;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

