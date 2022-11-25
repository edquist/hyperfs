#include <regex.h>      // regex functions

#include "ymdhms.h"
#include "month_idx.h"
#include "loggo.h"      // LOG

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

/* http_date_parse - parse http date header, aspiringly fast
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
long http_date_parse(const char *s)
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


/* now for the regex functions
 *
 * these are used for parsing timestamps out of generated file index
 * listings (html) ... the format is less precise and i'm a bit lazy
 * to do it all by hand like in http_date_parse
 *
 */

static
long get_tstamp1(const char *line)
{
	static regex_t reg;
	static int reg_compiled = 0;

	// 1. 2022-Oct-12 12:26:24

	regmatch_t m[8];
	const char *re = "> *([0-9]{4})-([A-Z][a-z]{2})-([0-9]{2})"
	                 " ([0-2][0-9]):([0-5][0-9])(:([0-5][0-9]))?[ <]";

	if (!reg_compiled) {
		LOG("[get_tstamp1: doing one-time reg compile]\n");
		if (regcomp(&reg, re, REG_EXTENDED)) {
			LOG("[get_tstamp1: regcomp fail]\n");
			return 0;
		}
		reg_compiled = 1;
	}

	if (regexec(&reg, line, 8, m, 0))
		return 0;

	struct ymdhmsz ts;

	ts.dt.year  = a2i4      (line + m[1].rm_so);
	ts.dt.month = month_idx (line + m[2].rm_so);
	ts.dt.day   = a2i2      (line + m[3].rm_so);
	ts.tm.hour  = a2i2      (line + m[4].rm_so);
	ts.tm.min   = a2i2      (line + m[5].rm_so);
	if (m[7].rm_so >= 0)
		ts.tm.sec = a2i2(line + m[7].rm_so);

	ts.zz = 0;  // XXX: probably not true; usually server's local TZ

	return epoch(&ts);
}


static
long get_tstamp2(const char *line)
{
	static regex_t reg;
	static int reg_compiled = 0;

	// 2. 2022-11-25 16:00

	regmatch_t m[8];
	const char *re = "> *([0-9]{4})-([01][0-9])-([0-3][0-9])"
	                 " ([0-2][0-9]):([0-5][0-9])(:([0-5][0-9]))?[ <]";

	if (!reg_compiled) {
		LOG("[get_tstamp2: doing one-time reg compile]\n");
		if (regcomp(&reg, re, REG_EXTENDED)) {
			LOG("[get_tstamp2: regcomp fail]\n");
			return 0;
		}
		reg_compiled = 1;
	}

	if (regexec(&reg, line, 8, m, 0))
		return 0;

	struct ymdhmsz ts;

	ts.dt.year  = a2i4      (line + m[1].rm_so);
	ts.dt.month = a2i2      (line + m[2].rm_so);
	ts.dt.day   = a2i2      (line + m[3].rm_so);
	ts.tm.hour  = a2i2      (line + m[4].rm_so);
	ts.tm.min   = a2i2      (line + m[5].rm_so);
	if (m[7].rm_so >= 0)
		ts.tm.sec = a2i2(line + m[7].rm_so);

	ts.zz = 0;  // XXX: probably not true; usually server's local TZ

	return epoch(&ts);
}


static
time_t get_tstamp3(const char *line)
{
	static regex_t reg;
	static int reg_compiled = 0;

	// 3. 13-Jan-2021 19:55

	regmatch_t m[8];
	const char *re = "> *([0-3][0-9])-([A-Z][a-z]{2})-([0-9]{4})"
	                 " ([0-2][0-9]):([0-5][0-9])(:([0-5][0-9]))?[ <]";

	if (!reg_compiled) {
		LOG("[get_tstamp3: doing one-time reg compile]\n");
		if (regcomp(&reg, re, REG_EXTENDED)) {
			LOG("[get_tstamp3: regcomp fail]\n");
			return 0;
		}
		reg_compiled = 1;
	}

	if (regexec(&reg, line, 8, m, 0))
		return 0;

	struct ymdhmsz ts;

	ts.dt.year  = a2i4      (line + m[3].rm_so);
	ts.dt.month = month_idx (line + m[2].rm_so);
	ts.dt.day   = a2i2      (line + m[1].rm_so);
	ts.tm.hour  = a2i2      (line + m[4].rm_so);
	ts.tm.min   = a2i2      (line + m[5].rm_so);
	if (m[7].rm_so >= 0)
		ts.tm.sec = a2i2(line + m[7].rm_so);

	ts.zz = 0;  // XXX: probably not true; usually server's local TZ

	return epoch(&ts);
}


long get_tstamp(const char *line)
{
	long ts;
	if ((ts = get_tstamp1(line))) return ts;
	if ((ts = get_tstamp2(line))) return ts;
	return    get_tstamp3(line);
}

/* vim: set noexpandtab sts=0 sw=8 ts=8: */

