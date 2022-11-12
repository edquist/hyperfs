#include <time.h>

#include "ymdhms.h"
#include "month_idx.h"

inline static
int isalpha(unsigned c)
{
	return (c | ' ') - 'a' <= 'z' - 'a';
}

inline static
int isnum(unsigned c)
{
	return c - '0' <= '9' - '0';
}

inline static
const char *skipsp(const char *s)
{
	while (*s == ' ')
		++s;
	return s;
}

inline static
int a2i(const char **sp)
{
	int val = 0;
	const char *s = *sp;
	if (isnum(*s))
		val += *s++ - '0';
	while (isnum(*s)) {
		val *= 10;
		val += *s++ - '0';
	}
	*sp = s;
	return val;
}

// signed version
inline static
int sa2i(const char **sp)
{
	switch (**sp) {
	case '-': ++*sp; return -a2i(sp);
	case '+': ++*sp; return  a2i(sp);
	default :      ; return  a2i(sp);
	}
}

// exactly 2 digits version
inline static
int a2i2(const char *s)
{
	// XXX: assumes (isnum(s[0]) && isnum(s[1]))

	return s[0] * 10 + s[1] - ('0' * 11);
}

// exactly 4 digits version
inline static
int a2i4(const char *s)
{
	return a2i2(s) * 100 + a2i2(s + 2);
}

/* date_parse - parse http date header, aspiringly fast
 *
 * "Last-Modified: Wed, 09 Nov 2022 11:12:50 GMT"
 *
 * essentially equivalent to:
 *
 *   const char *fmt = "%a, %d %b %Y %H:%M:%S GMT";
 *   struct tm ts;
 *   int tz;
 *   char *ts_start = strptime(dstring, fmt, &ts);
 *   sscanf(ts_start, "%d", &tz);
 *   return timegm(&ts) - tz * 36;  // 36 == 60 * 60 / 100
 *
 */
long date_parse(const char *s)
{
	if (isalpha(s[0]) && isalpha(s[1]) && isalpha(s[2]) && s[3] == ',')
		s += 4;

	s = skipsp(s);

	int day = a2i2(s) - 1;
	s += 2;
	if (*s++ != ' ')
		return -1;

	int month = month_idx(s);
	s += 3;
	if (*s++ != ' ')
		return -1;

	int year = a2i4(s);
	s += 4;
	if (*s++ != ' ')
		return -1;

	int hour = a2i2(s);
	s += 2;
	if (*s++ != ':')
		return -1;

	int min = a2i2(s);
	s += 2;
	if (*s++ != ':')
		return -1;

	int sec = a2i2(s);
	s += 2;
	if (*s++ != ' ')
		return -1;

	if (s[0] != 'G' || s[1] != 'M' || s[2] != 'T')
		return -1;

	int tz = 0;

	struct ymdhmsz ts = {{year, month, day}, {hour, min, sec}, tz};
	return epoch(&ts);
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

