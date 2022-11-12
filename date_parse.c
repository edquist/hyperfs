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

/* date_parse - aspiringly fast http date header parser
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

	int day = a2i(&s) - 1;
	s = skipsp(s);

	int month = month_idx(s);
	s += 3;
	s = skipsp(s);

	int year = a2i(&s);
	s = skipsp(s);

	int hour = a2i(&s);
	if (*s++ != ':')
		return -1;

	int min = a2i(&s);
	if (*s++ != ':')
		return -1;

	int sec = a2i(&s);
	s = skipsp(s);

	if (s[0] != 'G' || s[1] != 'M' || s[2] != 'T')
		return -1;
	int tz = 0;

	struct ymdhmsz ts = {{year, month, day}, {hour, min, sec}, tz};
	return epoch(&ts);
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

