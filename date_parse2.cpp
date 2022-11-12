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

long date_parse2(const char *s)
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

	int tz = sa2i(&s);

	struct ymdhmsz ts = {{year, month, day}, {hour, min, sec}, tz};
	return epoch(&ts);
}

#if 0
// parse YYYY-MM-DD
struct ymd ymd_parse(const char *s)
{
	const static struct ymd bad = {-1, -1, -1};

	int year = a2i(&s);
	if (*s++ != '-')
		return bad;

	int month = a2i(&s) - 1;
	if (*s++ != '-')
		return bad;

	int day = a2i(&s) - 1;

	struct ymd dt = {year, month, day};
	return dt;
}


long ymd_localtime(const char *s)
{
	struct ymd dt = ymd_parse(s);

	struct tm tm = {};
	tm.tm_mday = dt.day + 1;
	tm.tm_mon  = dt.month;
	tm.tm_year = dt.year - 1900;
	tm.tm_isdst = -1;

	return mktime(&tm);
}
#endif


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

