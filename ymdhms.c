#include "ymdhms.h"

int daynum(struct ymd *dt)
{
	// day index within year for each month
	static const int mdidx[12] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

	// wonkily find leap years since epoch
	int yx = dt->year - 1969;
	int leaps = yx / 4;
	if (yx % 4 == 3 && dt->month > 1)
		leaps++;

	return (yx - 1) * 365 + mdidx[dt->month] + dt->day + leaps;
}

int timeofday(struct hms *tm)
{
	return tm->hour * (60 * 60) + tm->min * 60 + tm->sec;
}

long epoch(struct ymdhmsz *ts)
{
	return daynum(&ts->dt) * (60 * 60 * 24)
	     + timeofday(&ts->tm)
	     - ts->zz * 36;   // 36 == 60 * 60 / 100
}


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

