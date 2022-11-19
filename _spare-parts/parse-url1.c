
char *parse_url1(char *url, struct hyperfs_state *state)
{
	char *p = strstr(url, "://");
	if (p) {
		*p = 0;
		if (strcmp(url, "http") != 0) {
			FAIL("Sorry, only http is supported\n");
		}
		state->proto = url;
		url = p + strlen("://");
	} else {
		state->proto = "http";  // still the only choice
	}

	state->host = url;
	p = strchr(url, '/');  // beginning of rootpath
	if (p) {
		state->rootpath = strdup(p);
		chompc(state->rootpath, '/');
		*p = 0;
		p = strchr(url, ':');  // port separator, if any
		if (p) {

		}
	} else {
		state->rootpath = "";
	}

	state->host     = url;     // TODO: parse
	state->port     = "80";
	state->port_num = 80;
	state->rootpath = "";
	state->sockfd   = -1;
	state->sockf    = NULL;

	return xasprintf("%s://%s:%s%s/", state->proto, state->host,
	                                  state->port,  state->rootpath);
}

/* vim: set noexpandtab sts=0 sw=8 ts=8: */

