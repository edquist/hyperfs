#include <time.h>

#include "ymdhms.h"
#include "month_idx.h"

inline static
bool isalpha(unsigned c)
{
    return (c | ' ') - 'a' <= 'z' - 'a';
}

inline static
bool isnum(unsigned c)
{
    return c - '0' <= '9' - '0';
}

inline static
void skipsp(const char *&s)
{
    while (*s == ' ')
        ++s;
}

// hrm, how will this compare to atoi?
inline static
int a2i(const char *&s)
{
    int val = 0;
    if (isnum(*s))
        val += *s++ - '0';
    while (isnum(*s)) {
        val *= 10;
        val += *s++ - '0';
    }
    return val;
}

// signed version
inline static
int sa2i(const char *&s)
{
    switch (*s) {
    case '-': return -a2i(++s);
    case '+': return  a2i(++s);
    default : return  a2i(s);
    }
}

long date_parse2(const char *s)
{
    if (isalpha(s[0]) && isalpha(s[1]) && isalpha(s[2]) && s[3] == ',')
        s += 4;

    skipsp(s);

    int day = a2i(s) - 1;
    skipsp(s);

    int month = month_idx(s);
    s += 3;
    skipsp(s);

    int year = a2i(s);
    skipsp(s);

    int hour = a2i(s);
    if (*s++ != ':')
        return -1;

    int min = a2i(s);
    if (*s++ != ':')
        return -1;

    int sec = a2i(s);
    skipsp(s);

    int tz = sa2i(s);

    return ymdhmsz(ymd(year, month, day), hms(hour, min, sec), tz).epoch();
}

// parse YYYY-MM-DD
ymd ymd_parse(const char *s)
{
    int year = a2i(s);
    if (*s++ != '-')
        return ymd(-1, -1, -1);

    int month = a2i(s) - 1;
    if (*s++ != '-')
        return ymd(-1, -1, -1);

    int day = a2i(s) - 1;

    return ymd(year, month, day);
}


long ymd_localtime(const char *s)
{
    ymd dt = ymd_parse(s);

    struct tm tm = {};
    tm.tm_mday = dt.day + 1;
    tm.tm_mon  = dt.month;
    tm.tm_year = dt.year - 1900;
    tm.tm_isdst = -1;

    return mktime(&tm);
}

