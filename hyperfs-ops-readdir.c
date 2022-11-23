#include <fuse.h>           // struct fuse_file_info, fuse_fill_dir_t
#include <regex.h>          // regmatch_t
#include <errno.h>          // EIO
#include <string.h>         // memchr

#include "loggo.h"          // LOG
#include "hyperfs-state.h"  // struct hyperfs_state, get_hyperfs_state
#include "hyperfs-cache.h"  // add_pathbuf
#include "escape.h"         // path_is_escaped
#include "hyperfs-get.h"    // hyperget
#include "ministat.h"       // struct ministat


static inline
size_t min(size_t a, size_t b) { return a <= b ? a : b; }

static inline
int streq(const char *s1, const char *s2) { return strcmp(s1, s2) == 0; }

static
int single_path_component(const char *href, const char *end)
{
	const char *found = memchr(href, '/', end - href);
	return !found || found == end - 1;  // no slash or slash at end
}

static
int dot_dir(const char *s)
{
	// keeping this simple for readability ...

	if (s[0] != '.')
		return 0;

	return streq(s, ".")
	    || streq(s, "./")
	    || streq(s, "..")
	    || streq(s, "../");
}

static
int link_check(const char *href, const char *end)
{
	return href < end                       // no empty path
	    && href[0] != '/'                   // no links to root '/'
	    && !dot_dir(href)                   // no "." or ".."
	    && single_path_component(href, end) // no "abc/def" entries
	    && !memchr(href, '?', end - href);  // no url query params!
}

static
char *maybe_take_path(char *linebuf, regmatch_t href)
{
	char *s = linebuf + href.rm_so;
	char *e = linebuf + href.rm_eo;
	*e = '\0';  // it's easier this way...
	if (s[0] == '.' && s[1] == '/')
		s += 2;  // apache does this if file has a ':'

	if (link_check(s, e)) {
		if (e[-1] == '/') {
			*--e = '\0';
			*--s = '/';  // OK, overwrite opening '"'
		}
		if (path_is_escaped(s))
			e = decode_escaped(s, s);  // in-place
		char *p = add_pathbuf(s, e - s + 1);
		if (!p) {
			LOG("[maybe_take_path: add_pathbuf alloc failure]\n");
			return NULL;
		}
		return p;
	} else {
		// TODO: add as symlink
		return NULL;
	}
}

static
regmatch_t get_href(const char *line)
{
	const static regmatch_t nope = {-1, -1};
	static regex_t reg;
	static int reg_compiled = 0;

	regmatch_t m[2];
	const char *re = "<a [^>]*href=\"([^\"]+)\"";
	if (!reg_compiled) {
		LOG("[get_href: doing one-time reg compile]\n");
		if (regcomp(&reg, re, REG_EXTENDED)) {
			LOG("[get_href: regcomp fail]\n");
			return nope;
		}
		reg_compiled = 1;
	}
	if (regexec(&reg, line, 2, m, 0))
		return nope;
	else
		return m[1];
}

static
void mark_dir(const char *rootpath, const char *path, const char *name)
{
	char buf[4096];
	if (name[0] != '/')
		return;

	if (strlen(rootpath) + strlen(path) + strlen(name) >= sizeof buf) {
		LOG("[mark_dir: oversized path; not marking]\n"
		    "[mark_dir: path = [%s] ]\n"
		    "[mark_dir: name = [%s] ]\n", path, name);
		return;
	}
	char *p = buf;
	p = stpcpy(p, rootpath);
	p = stpcpy(p, path);
	p = stpcpy(p, name);

	if (!get_cached_path_info_p(buf)) {
		// add it if we don't already have it
		struct ministat st = {.type = S_IFDIR};
		int ret = set_cached_path_info(buf, &st);
		if (ret < 0) {
			LOG("[mark_dir: set_cached_path_info failed]\n");
			perror("hsearch_r");
		}
	}
}

static
char *cache_hyperdents(struct hyperfs_state *remote, const char *path)
{
	size_t input_rem;
	FILE *sockf = hyperget(remote, path, &input_rem);
	if (!sockf) {
		LOG("[cache_hyperdents: hyperget failed]\n");
		return NULL;
	}

	char *pstart = get_pathbuf(0);
	char linebuf[4096];
	int readsize = min(input_rem + 1, sizeof linebuf);
	int n = 0;
	while (input_rem && fgets(linebuf, readsize, sockf)) {
		size_t line_len = strlen(linebuf);
		regmatch_t href = get_href(linebuf);
		if (href.rm_so >= 0) {
			char *name = maybe_take_path(linebuf, href);
			if (name) {
				n++;
			}
		}
		if (line_len && linebuf[line_len - 1] == '\n') {
			// "Situation Normal"
			input_rem -= line_len;
		} else if (line_len == input_rem) {
			// EOF hit, without EOL, also OK
			input_rem = 0;
			break;
		} else {
			// NUL in input stream; those jerks.
			// No way to tell how long this line actually was,
			// so treat this as the end and cut off the input.
			LOG("[cache_hyperdents: unexpected NUL in input]\n");
			addpath("");
			fclose(sockf);
			return pstart;
		}
		readsize = min(input_rem + 1, sizeof linebuf);
	}

	LOG("[cache_hyperdents: read and cached %d dir entries]\n", n);
	addpath("");
	fclose(sockf);

	char *name = pstart;
	while (*name) {
		if (name[0] == '/')
			mark_dir(remote->rootpath, path, name);
		name += strlen(name) + 1;
	}

	return pstart;
}


int hyperfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                    off_t offset, struct fuse_file_info *fi)
{
	LOG("[readdir: '%s' offset=[%ld] ]\n", path, offset);

	struct hyperfs_state *remote = get_hyperfs_state();

	struct ministat *ms = get_cached_path_info_p(path);
	if (!ms) {
		LOG("[readdir: uh, path isn't in cache]\n");
		return -EBADFD;
	}
	if (!ms->hyperdir) {
		ms->hyperdir = cache_hyperdents(remote, path);
		if (!ms->hyperdir) {
			LOG("[readdir: returning EIO]\n");
			return -EIO;  // or maybe EACCESS depending on error
		}
	}

	char *name = ms->hyperdir;
	struct stat st;
	memset(&st, 0, sizeof st);

	st.st_mode = S_IFDIR;
	if (filler(buf, ".",  &st, 0) ||
	    filler(buf, "..", &st, 0))
		return -1; // ???

	while (*name) {
		st.st_mode = name[0] == '/' ? S_IFDIR : S_IFREG;
		if (name[0] == '/')
			name++;

		LOG("[readdir: sending '%s']\n", name);
		if (filler(buf, name, &st, 0))
			break;
		name += strlen(name) + 1;
	}

	return 0;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

