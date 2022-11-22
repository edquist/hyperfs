#include <ctype.h>   // isxdigit
#include <string.h>  // strncmp

static
int needs_escape(unsigned char c)
{
	switch (c) {
	case '"':
	case '#':
	case '%':
	case '&':
	// case '/':  // can't appear in pathname
	case '<':
	case '>':
	case '?':
	case '[':
	case '\\':
	case ']':
	case '^':
	case '`':
	case '{':
	case '|':
	case '}': return 1;
	default : return c <= ' ' || c > '~';
	}
}

// return total length needed if any chars need escape, otherwise 0
int path_needs_escape(const char *s)
{
	const char *p = s;
	int ecount = 0;
	while (*p)
		if (needs_escape(*p++))
			ecount++;
	return ecount ? p - s + 2 * ecount : 0;
}

static
char hexnib(int n)
{
	return n < 10 ? '0' + n : ('a' - 10) + n;
}

char *escape_raw(char *dest, const char *raw)
{
	const unsigned char *u = (const unsigned char *) raw;
	for (; *u; u++) {
		if (needs_escape(*u)) {
			*dest++ = '%';
			*dest++ = hexnib(*u / 16);
			*dest++ = hexnib(*u % 16);
		} else {
			*dest++ = *u;
		}
	}
	*dest = '\0';
	return dest;
}

static
int hexdecode_nib(char nib)
{
	return nib <= '9' ? nib - '0' : (nib | ' ') - ('a' - 10);
}

static
unsigned char hexdecode(const char *hex)
{
	return (hexdecode_nib(hex[0]) << 8) | hexdecode_nib(hex[1]);
}

int path_is_escaped(const char *s)
{
	return strchr(s, '%') || strchr(s, '&');
}

char *decode_escaped(char *dest, const char *s)
{
	while (*s) {
		if (*s == '%' && isxdigit(s[1]) && isxdigit(s[2])) {
			*dest++ = hexdecode(s);
			s += 3;
		} else if (*s == '&' && strncmp(s + 1, "amp;", 4) == 0) {
			*dest++ = '&';
			s += 5;
		} else {
			*dest++ = *s++;
		}
	}
	*dest = '\0';
	return dest;
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

